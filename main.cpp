#include "osal.h"
#include "serialport.h"
#include "datasocket.h"
#include "hidtemp.h"
#include "filesysmgr.h"
#include "eimdata.h"
#include "boardctl.h"
#include "heartsocket.h"



Serialport *gpsport=NULL;
DataSocket *g_datasock=NULL;
DataSocket *g_gpssock=NULL;
CFilesys_mgr *g_datafs=NULL;
temperdev *g_hidtemp=NULL;
EIMDATA *g_eimdata=NULL;
boardctl *g_myboard=NULL;
FILE *fplog=NULL;
std::list<DataSocket *>socklist;
int server_mode = 0;
cmutex sockmutex;


#define LOCAL_ADDR 0x0001



char *svr_ipaddr;
unsigned int svr_port;

enum
{
	data_server,
	heartbeat_server,
};

#define SEND_UNIT   496

char senddata[SEND_UNIT*2];
char buffdata[SEND_UNIT*2];

void on_datasock_read(struct bufferevent *cb, void *ctx);
void on_sock_write(struct bufferevent *cb, void *ctx);
void on_socket_event(struct bufferevent *bev, short ev, void *ctx);
void handle_timeout(int nSock, short sWhat, void * pArg);
void set_socket_server_start(struct event_base *mybase, int port, int type);
void do_accept(evutil_socket_t listener, short event, void *arg);

struct event_base *my_evbase;

void illegal_inst_handler(int sig)
{
	void *array[10];
	size_t size;
	char **strframe ;
	size_t i;
	
	if( sig==SIGKILL || sig==SIGTERM ||sig==SIGQUIT ||sig==SIGHUP||sig==SIGINT)
	{
		printf("get signal: %d\n", sig);
		if(fplog)			
			fprintf(fplog, "-----------------------------------------------\n");
		size = backtrace (array, 10);
		strframe= backtrace_symbols (array, size);
		printf("print call frame now:\n");
	  	for(i = 0; i < size; i++)
		{
	    		printf("frame %d -- %s\n", i, strframe[i]);
			if(fplog)		
				fprintf(fplog, "frame %d -- %s\n", i, strframe[i]);
	  	}
		if(strframe)
		{
			free(strframe);
			strframe = NULL;
		 }
		if(fplog)
		{
			fprintf(fplog, "-----------------------------------------------\n");
			fflush(fplog);
		}
		if(g_eimdata)
		{
			g_eimdata->release();
			delete g_eimdata;
		}
		if(g_datafs)
			delete g_datafs;		

		exit(0);
	}
	else if(sig == SIGILL  || sig == SIGSEGV)
	{
		printf("get illegal instruction or segmation fault: %d\n", sig);
		if(fplog)			
			fprintf(fplog, "-----------------------------------------------\n");
		size = backtrace (array, 10);
		strframe= backtrace_symbols (array, size);
		printf("print call frame now:\n");
	  	for(i = 0; i < size; i++)
		{
	    		printf("frame %d -- %s\n", i, strframe[i]);
			if(fplog)		
				fprintf(fplog, "frame %d -- %s\n", i, strframe[i]);
	  	}
		if(strframe)
		{
			free(strframe);
			strframe = NULL;
		 }
		if(fplog)
		{
			fprintf(fplog, "-----------------------------------------------\n");
			fflush(fplog);
		}
	}

	return ;
}

int main(int argc, char *argv[])
{
	int fd=0;
	int devfd;
	int nret=0;
	int cnt=0;
	u32 local_address=0;
	char *ipaddr;
	unsigned int gps_port=0;
	struct timeval tTimeout = {10, 0};
	struct event evTimeout;
	
	int times=0;
	int nbytes=0;
	int packcnt=0;
	int gpslen=0;
	int index=0,slice_len;
	int nodelay=1;
	volatile int testdelay=0;
	int bufflen=0;
	int virgps=10101010;
	//int server_mode = 0;
	

	//if((argc == 2 && strcmp(argv[1],"-s")) 
	if(argc < 3)
	{
		printf("Usage: %s [ipaddr] [dataport]\n example:%s  -[c/s] 192.168.1.1 8001 \n",argv[0], argv[0]);
		printf("Please specify IP addr and Port\n");
		return -1;
	}
	fplog=fopen("thunderlog.txt","w+");
	assert(fplog);
	printf("fplog: 0x%x \n",fplog);
	fprintf(fplog,"now, new %s %s %s start\n", argv[0],argv[1],argv[2], argv[3]);
	fflush(fplog);

	
	nret=evthread_use_pthreads();
	printf("evthread_use_pthreads: %d\n", nret);

	if(strcmp(argv[1], "-s") == 0)
		server_mode=1;
	else
		server_mode=0;

	gpsport = new Serialport();
	if(gpsport->init("/dev/ttymxc0", 1, 115200, 1) != 0)
	{
		printf("start dev tty failed\n");
		delete gpsport;
		return -1;
	}
	
	devfd=open("/dev/eimfpga",O_RDWR);	
	if(devfd < 0)	
	{		
		printf("open EIM FPGA device failed\n");	
		delete gpsport;
		return -1;	
	}

	g_myboard=new boardctl();

	g_datafs=new CFilesys_mgr("/dev/sda1","/eim/data",0x80000000);//512GB
	g_eimdata=new EIMDATA();


	g_hidtemp=new temperdev();
	g_hidtemp->init("/dev/hiddev0", g_myboard);
		
	signal(SIGPIPE, SIG_IGN);
	signal(SIGILL,  illegal_inst_handler);
	signal(SIGINT,	illegal_inst_handler);
	signal(SIGTERM, illegal_inst_handler);
	signal(SIGQUIT, illegal_inst_handler);
	signal(SIGHUP, illegal_inst_handler);
	
	//svr_ipaddr=argv[2];
	//svr_port=atoi(argv[3]);
	//gps_port=atoi(argv[3]);
	times=0;	
	cnt=0;



	
	my_evbase=event_base_new();

	if(server_mode)
	{
		int port=atoi(argv[2]);
		printf("You specify server listen on  port: %d\n", port);
		set_socket_server_start(my_evbase, port,  data_server);
		set_socket_server_start(my_evbase, port+1, heartbeat_server);
	}
	else
	{
		g_datasock=new DataSocket();
		g_datasock->init(argv[2],argv[3], my_evbase);
		g_datasock->setcb(on_datasock_read, on_sock_write, on_socket_event);
	}

#if 0
	g_gpssock=new DataSocket();
	if(g_gpssock->init(argv[1],argv[3], my_evbase) < 0)
	{
		printf("create g_gpssock failed\n");		
	}
	g_gpssock->setcb(NULL, on_sock_write, on_socket_event);
#endif

	nbytes=0;
	//g_eimdata->init(devfd, g_datafs, g_datasock);	
	g_eimdata->init(devfd, g_datafs, g_myboard);

	gpsport->set_boardctl(g_myboard);

	g_myboard->init(NULL);
	g_myboard->set_data_interface(g_eimdata);
	
	event_assign(&evTimeout, my_evbase, -1, EV_PERSIST, handle_timeout, NULL);
	 evtimer_add(&evTimeout, &tTimeout);
	 
	event_base_dispatch(my_evbase);



	while(1)		
	{		
		usleep(100000);			
	}

	return 0;
}

void on_datasock_read(struct bufferevent *bev, void *ctx)
{
	char data[100];
	DataSocket *psock=(DataSocket *)ctx;
	struct evbuffer *input=bufferevent_get_input(bev);
	size_t len=evbuffer_get_length(input);
	
	printf("%s 0x%x len: %d \n", __func__, ctx, len);
	evbuffer_remove(input, data, 100);

	host_request_proc(g_myboard,data);
	printf("%x %x %x %x\n",data[0],data[1],data[2],data[3]);
	
}


void on_heartsock_read(struct bufferevent *bev, void *ctx)
{	
	HeartSocket *psock=(HeartSocket *)ctx;	
    	struct timeval tTimeout = {10, 0};

	evtimer_del(&psock->evtimer);
	
	event_assign(&psock->evtimer, base, -1, EV_PERSIST, handle_timeout, NULL);
	evtimer_add(&psock->evtimer, &tTimeout);	
}


void  on_sock_write(struct bufferevent *bev, void *ctx)
{
	DataSocket *psock=(DataSocket *)ctx;	
//	printf("on_sock_write\r\n");
//	usleep(1000000);	
}

void on_socket_event(struct bufferevent *bev, short ev, void *ctx)
{
	DataSocket *psock=(DataSocket *)ctx;	
	//printf("%s sock:0x%x event:0x%x\n", __func__, psock, ev);
	if (ev & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) 
	{
		printf("%s socket event: 0x%x sock:0x%x\n", __func__, ev, psock);
		if(psock)
		{
			psock->release();	
			if(server_mode &&  psock == g_datasock)
			{
				g_myboard->set_data_sock(NULL);
				sockmutex.lock();
				psock = socklist.front();
				printf("front: 0x%x datasock:0x%x\n", psock, g_datasock);
				delete g_datasock;
				g_datasock=NULL;
				socklist.pop_front();
					
				printf("list size: 0x%x\n", socklist.size());
			
				if(!socklist.empty())
				{
					psock = socklist.front();
					g_datasock=psock;
					g_myboard->set_data_sock(psock); 
				}
				sockmutex.unlock();
			}
		}
	}
	if(ev & BEV_EVENT_CONNECTED)
	{
		printf("socket %d connect OK\n", psock->m_sockfd);
	}
	if(ev & BEV_EVENT_READING)
		printf("on_socket_event BEV_EVENT_READING:%d\n", ev);
	if(ev & BEV_EVENT_WRITING)
		printf("on_socket_event BEV_EVENT_WRITING:%d\n", ev);
	if(ev & BEV_EVENT_TIMEOUT)
		printf("on_socket_event BEV_EVENT_TIMEOUT:%d\n", ev);
}


void on_heartsock_event(struct bufferevent *bev, short ev, void *ctx)
{
	DataSocket *psock=(DataSocket *)ctx;	
	//printf("%s sock:0x%x event:0x%x\n", __func__, psock, ev);
	if (ev & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) 
	{
		printf("%s socket event: 0x%x sock:0x%x\n", __func__, ev, psock);
		if(psock)
		{
			psock->release();	
			if(server_mode &&  psock == g_datasock)
			{
				g_myboard->set_data_sock(NULL);
				sockmutex.lock();
				psock = socklist.front();
				printf("front: 0x%x datasock:0x%x\n", psock, g_datasock);
				delete g_datasock;
				g_datasock=NULL;
				socklist.pop_front();
					
				printf("list size: 0x%x\n", socklist.size());
			
				if(!socklist.empty())
				{
					psock = socklist.front();
					g_datasock=psock;
					g_myboard->set_data_sock(psock); 
				}
				sockmutex.unlock();
			}
		}
	}
	if(ev & BEV_EVENT_CONNECTED)
	{
		printf("socket %d connect OK\n", psock->m_sockfd);
	}
	if(ev & BEV_EVENT_READING)
		printf("on_socket_event BEV_EVENT_READING:%d\n", ev);
	if(ev & BEV_EVENT_WRITING)
		printf("on_socket_event BEV_EVENT_WRITING:%d\n", ev);
	if(ev & BEV_EVENT_TIMEOUT)
		printf("on_socket_event BEV_EVENT_TIMEOUT:%d\n", ev);
}

void handle_timeout(int nSock, short sWhat, void * pArg)
{
	DataSocket *psock;
	printf("g_datasock 0x%x status:0x%x\n", g_datasock, ((!g_datasock)?0:g_datasock->get_status()));
	if(g_datasock && (g_datasock->get_status() != sock_dataing))
	{
	}
}

void set_socket_server_start(struct event_base *mybase, int port, int type)
{
	evutil_socket_t listener;  
	struct sockaddr_in sin;  
	struct event *listener_event;  

	sin.sin_family = AF_INET;  
	sin.sin_addr.s_addr = 0;  
	sin.sin_port = htons(port);  

	listener = socket(AF_INET, SOCK_STREAM, 0);  
	evutil_make_socket_nonblocking(listener);  

	int one = 1;  
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));  

	if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0)  
	{  
		printf("bind error\n");  
		return;  
	}  
	if (listen(listener, 16)<0)  
	{  
		printf("listen error\n");  
		return;  
	}  

	if(type == data_server)
	{
		listener_event = event_new(mybase, listener, EV_READ|EV_PERSIST, do_accept, (void*)mybase);  
	}
	else if(type == heartbeat_server)
	{
		listener_event = event_new(mybase, listener, EV_READ|EV_PERSIST, do_accept_heart, (void*)mybase);  
	}
	event_add(listener_event, NULL);  

	return ;
}

void do_accept(evutil_socket_t listener, short event, void *arg)  
{  
    struct event_base *base = (struct event_base *)arg;  
    struct sockaddr_storage ss;  
    socklen_t slen = sizeof(ss);  
	int optval, ret;
	
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);  
    if (fd < 0)  
    {  
        printf("accept error");  
    }  
    else if (fd > FD_SETSIZE)  
    {  
    	printf("%s fd > FD_SETSIZE\n",__func__);
        close(fd);  
    }  
    else  
    {
    	printf("accept new fd: %d\n", fd);
	ret =setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval));
	if(ret != 0)
		printf("SO_KEEPALIVE failed\n");
		

	int keepIdle=20; //idle time before trigger
	ret = setsockopt(fd, SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle));
	if(ret != 0)
		printf("TCP_KEEPIDLE failed\n");

	int keepInterval=60; //interval time between trigger
	ret =setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval));
	if(ret != 0)
		printf("TCP_KEEPINTVL failed\n");

	int keepCount=3; //connect out try count
	ret =setsockopt(fd, SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount));
	if(ret != 0)
		printf("TCP_KEEPCNT failed\n");
		
        struct bufferevent *bev;  

		//DataSocket *g_tmpsock=new DataSocket();
	DataSocket *t_clientsock=new DataSocket();
	printf("new DataSocket: 0x%x\n", t_clientsock);
       	evutil_make_socket_nonblocking(fd);  
	
	t_clientsock->init(fd, base);
	t_clientsock->setcb(on_datasock_read, on_sock_write, on_socket_event);

	sockmutex.lock();
	socklist.push_back(t_clientsock);
	printf("list size: 0x%x\n", socklist.size());
	if(socklist.size() == 1)
	{
		g_datasock=t_clientsock;
		g_myboard->set_data_sock(g_datasock);
	}
	sockmutex.unlock();
    }  
}  


void do_accept_heart(evutil_socket_t listener, short event, void *arg)  
{  
    struct event_base *base = (struct event_base *)arg;  
    struct sockaddr_storage ss;  
    socklen_t slen = sizeof(ss);  
    struct timeval tTimeout = {10, 0};
	
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);  
    if (fd < 0)  
    {  
        printf("accept error");  
    }  
    else if (fd > FD_SETSIZE)  
    {  
    	printf("%s fd > FD_SETSIZE\n",__func__);
        close(fd);  
    }  
    else  
    {  
    	printf("accept new fd: %d\n", fd);
	HeartSocket *t_clientsock=new HeartSocket();
	printf("new HeartSocket: 0x%x\n", t_clientsock);
       	evutil_make_socket_nonblocking(fd);  
	
	t_clientsock->init(fd, base);
	t_clientsock->setcb(on_heartsock_read, NULL, on_heartsock_event);
	
	event_assign(&t_clientsock->evtimer, base, -1, EV_PERSIST, handle_timeout, NULL);
	evtimer_add(&t_clientsock->evtimer, &tTimeout);
    }  
}  


