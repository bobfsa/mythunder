#include "osal.h"
#include "datasocket.h"


int DataSocket::init(int sockfd, int port,size_t length)
{
	m_brunning = 1;
	m_msgsize=length;
	m_sockstatus = 1;
	start();	
	detach();
	m_sockfd=sockfd;
	m_port=port;
	return 0;
}

int DataSocket::init(const char *ip, const char *port, struct event_base *evbase)
{
	struct    sockaddr_in    t_addr;  

	if(evbase == NULL)
	{
		printf("create evbase: 0x%x\n",evbase);
	}

	printf("addr sets: 0x%x 0x%x\n", inet_addr(ip),atoi(port));
	m_destip=(char *)ip;
	m_destport=(char *)port;
	
	bzero(&t_addr,sizeof(struct    sockaddr_in));  
	t_addr.sin_family = AF_INET;  
	t_addr.sin_port = htons(atoi(port));  
	t_addr.sin_addr.s_addr = inet_addr(ip);//按IP初始化   

	printf("%s start connect: %s %s\n", __func__, ip, port);

	m_bufev=bufferevent_socket_new(evbase,  -1,  BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);

	//bufferevent_setcb(m_bufev, NULL, NULL, NULL, this);
	//bufferevent_enable(m_bufev, EV_READ|EV_WRITE|EV_ET);
	
	printf("start buffevent socket connect\n");
	m_evbase=evbase;
	if(bufferevent_socket_connect(m_bufev, (struct sockaddr *)&t_addr, sizeof(struct sockaddr)))
	{
		m_sockstatus=sock_init_fail;
		bufferevent_free(m_bufev);
		m_bufev=NULL;
		printf("bufferevent_socket_connect failed\n");
		return -1;
	}
	printf("bufferevent_socket_connect OK\n");
	m_sockstatus = sock_init_nodata;
	m_brunning = 1;
	start();
	detach();	
	
	//event_base_dispatch(m_evbase);	
	return 0;
}

int DataSocket::restart()
{
	int ret=0;
	struct    sockaddr_in    t_addr;  

	//stop();
	//m_brunning = 0;
	//m_sockstatus = sock_uninit;
	
	//evutil_closesocket(bufferevent_getfd(m_bufev));
//	bufferevent_free(m_bufev);
//	m_bufev=NULL;
//	usleep(500000);	

	//close(bufferevent_getfd(m_bufev));
//	usleep(5000000);
	printf("try %s\n", __func__);
	bzero(&t_addr,sizeof(struct    sockaddr_in));  
	t_addr.sin_family = AF_INET;  
	t_addr.sin_port = htons(atoi(m_destport));  
	t_addr.sin_addr.s_addr = inet_addr(m_destip);//按IP初始化   
	
	m_bufev=bufferevent_socket_new(m_evbase,  -1,  BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);	
	if((ret=bufferevent_socket_connect(m_bufev, (struct sockaddr *)&t_addr, sizeof(struct sockaddr)))!=0)
	{
		m_sockstatus=sock_uninit;
		bufferevent_free(m_bufev);
		m_bufev=NULL;
		printf("bufferevent_socket_connect failed: %d\n",ret);
		return -1;
	}
	printf("bufferevent_socket_reconnect OK\n");
	m_sockstatus = sock_init_nodata;
	m_brunning = 1;
	start();
	detach();	
	return 0;
}


int DataSocket::setcb(bufferevent_data_cb cb1, bufferevent_data_cb cb2, bufferevent_event_cb cb3)
{
	if(m_bufev)
	{
		bufferevent_setcb(m_bufev, cb1, cb2, cb3, this);
		bufferevent_enable(m_bufev, EV_READ|EV_WRITE|EV_PERSIST);
	}
}

int DataSocket::submit(char *buff, size_t size)
{
	//printf("%s bufev:0x%x data:0x%x %d\n", __func__, m_bufev, buff, size);
	if(m_bufev)
	{
		return bufferevent_write(m_bufev, buff, size);
	}
	else 
		return 0;
}

int DataSocket::get_status()
{
	return m_sockstatus;
}

void DataSocket::release()
{
	stop();
	m_brunning = 0;
	m_sockstatus = sock_uninit;

	bufferevent_free(m_bufev);
	m_bufev=NULL;
	usleep(1000000);	
}




extern char readdata[];
extern char senddata[];
extern char buffdata[];


void *DataSocket::sub_routine(void)
{
#define SOCKDATA_LEN 28800
#define THREAD_SEND_UNIT 14400

	int slice_len=0,start=0;
	int nret=0;
	static char thread_readdata[SOCKDATA_LEN];
	char *pmsg=NULL;
	dataitem *pitem;
	int gpslen=0,index=0;
	int bufflen=0;
	char *msg;
	int nbytes;

	printf("%s start sockfd:%d %d\n",__func__,m_sockfd, m_port);
#if 0
	bufflen = 0;
	while (m_brunning)	
	{	
		msg_mutex.lock();
		if(! m_msgqueue.empty())
		{			
			pitem=m_msgqueue.front();
			//memcpy(buf, msg, m_msgsize);
			memcpy(thread_readdata, pitem->buff, pitem->length);
			nbytes=pitem->length;
			//printf("rx data: 0x%x 0x%x\n", nbytes, thread_readdata);
			m_msgqueue.pop();
			msg_mutex.unlock();

			delete pitem->buff;
			delete pitem;			
		}
		else
		{
			msg_mutex.unlock();
			usleep(1000);
			continue ;
		}
	#if 0	
		index=0;
		gpslen=0;

		while(nbytes>0)
		{
			if(bufflen>0)
			{
				memcpy(&senddata[gpslen], buffdata, bufflen);
				memcpy(&senddata[gpslen+bufflen], &thread_readdata[index], THREAD_SEND_UNIT-bufflen);
				index+=(THREAD_SEND_UNIT-bufflen);				
				slice_len=THREAD_SEND_UNIT+gpslen;
				nbytes-=(THREAD_SEND_UNIT-bufflen);		
				bufflen=0;
			}
			else
			{
				if(nbytes>=THREAD_SEND_UNIT)
				{
					memcpy(&senddata[gpslen], &thread_readdata[index], THREAD_SEND_UNIT);
					index+=THREAD_SEND_UNIT;				
					slice_len=THREAD_SEND_UNIT+gpslen;
					nbytes-=THREAD_SEND_UNIT;						
				}
				else
				{
					memcpy(buffdata, &thread_readdata[index], nbytes);
					index+=nbytes;	
					bufflen=nbytes;
					slice_len=0;
					nbytes=0;				
				}		
			}
			
			if(slice_len>0)
			{				
				nret=send(m_sockfd, senddata, slice_len, 0);
				if(nret != slice_len)				
					printf("sock send err: 0x%x 0x%x\n", slice_len, nret);							
				if(nret < 0)			
				{				
					printf("Socket error %d, Need Re-connect\n");				
					close(m_sockfd);		
					m_sockfd=-1;
					m_sockstatus=0;
					break;			
				}		
			}
		}	
	#endif
	
		nret=send(m_sockfd, thread_readdata, nbytes, 0);
		if(nret != nbytes)
			printf("sock send err: 0x%x 0x%x\n", nbytes, nret);							
		//else if(nbytes <= 32)
		//	printf("%d socket len: %d\n", m_sockfd, nbytes);
			

		if(nret < 0)			
		{				
			printf("Socket error %d, Need Re-connect\n");				
			close(m_sockfd);		
			m_sockfd=-1;
			m_sockstatus=0;
			break;			
		}	
		
		if(m_sockstatus == 0)
			break;
	}
#endif

#if 0
	if(nbytes == sizeof(thunderctl_msg))
		printf("Recv thunderctl_msg\n");
	else
		printf("Recv something, but not thunderctl_msg\n");

				
	pctlmsg=(thunderctl_msg *)buf;
	if(pctlmsg->ctl_dir != DIREC_PC2BOARD)
		printf("Rx error msg \n");

	switch(pctlmsg->ctl_type)
	{
		case BOARD_RESET:	
		{ 					
			//extern void reboot();
			printf("Recv Board Reset msg\n");
			close(sockdatafd);
			event_del(sock_ev_read);
			
			usleep(1000000);
			//reboot();					
			system("/sbin/reboot");
			break;
		}
		case BOARD_CONFIG:
		{
			printf("Recv BOARD_CONFIG msg\n");
			break;
		}
		case BOARD_UPDATE:
		{
			printf("Recv BOARD_UPDATE msg\n");
			break;			
		}
		case BOARD_THUNDER_PARAM_CONFIG:
		{
			printf("Recv BOARD_THUNDER_PARAM_CONFIG msg\n");
			break;			
		}
		case BOARD_CHECK_BOARD_STATUS:
		{
			printf("Recv BOARD_CHECK_BOARD_STATUS msg\n");
			break;			
		}
		case BOARD_CHECK_THUNDER:
		{
			printf("Recv BOARD_CHECK_THUNDER msg\n");
			break;			
		}
		case BOARD_DOWNLOAD_THUNDER_DATA:
		{
			thunderctl_proc(pctlmsg, fd);
			printf("Recv BOARD_CONFIG msg\n");
			break;			
		}						
	}	
#endif
	printf("DataSocket sub_routine exit \r\n");
	
	return NULL;
}

                  
