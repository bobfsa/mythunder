#include "osal.h"
#include "serialport.h"
#include "datasocket.h"
#include "hidtemp.h"
#include "filesysmgr.h"
#include "eimdata.h"
#include "boardctl.h"




Serialport *gpsport=NULL;


CFilesys_mgr *g_datafs=NULL;

EIMDATA *g_eimdata=NULL;






#define LOCAL_ADDR 0x0001



char *svr_ipaddr;
unsigned int svr_port;


#define SEND_UNIT   496

char senddata[SEND_UNIT*2];
char buffdata[SEND_UNIT*2];

void on_datasock_read(struct bufferevent *cb, void *ctx);
void on_gpssock_write(struct bufferevent *cb, void *ctx);
void on_socket_event(struct bufferevent *bev, short ev, void *ctx);
void handle_timeout(int nSock, short sWhat, void * pArg);

struct event_base *my_evbase;

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
	

	if(argc < 2)
	{
		printf("Usage: newgun [tty]\n example:./eimgps  /dev/ttymxc0\n");
		printf("Please specify DSP Port TTY\n");
		return -1;
	}


	printf("You specify the Recv TTY %s \n", argv[1]);

	
	gpsport = new Serialport();
	if(gpsport->init(argv[1], 1, 9600, 1) != 0)
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

	

	g_datafs=new CFilesys_mgr("/dev/sda1","/eim/data",0x80000000);//512GB
	g_eimdata=new EIMDATA();



	signal(SIGPIPE, SIG_IGN);


	nbytes=0;
	//g_eimdata->init(devfd, g_datafs, g_datasock);	
	g_eimdata->init(devfd, g_datafs, (boardctl *)NULL);

	//gpsport->set_boardctl(g_myboard);

	//g_myboard->init(g_datasock);
	
	//event_assign(&evTimeout, my_evbase, -1, EV_PERSIST, handle_timeout, NULL);
	// evtimer_add(&evTimeout, &tTimeout);
	 
	//event_base_dispatch(my_evbase);

	while(1)		
	{		
		usleep(100000);			
	}

	return 0;
}

void on_datasock_read(struct bufferevent *bev, void *ctx)
{

}
void  on_gpssock_write(struct bufferevent *bev, void *ctx)
{

}

void on_socket_event(struct bufferevent *bev, short ev, void *ctx)
{

}

void handle_timeout(int nSock, short sWhat, void * pArg)
{
}
