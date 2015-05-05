#include "osal.h"
#include "serialport.h"
#include "gunparse.h"


#define _THUNDER_

//const char *tgtGPS="$GPRMC";
//const char *locationGPS="$GPRMC";
const char *locationGPS="$AHRS:";

enum
{
	gps_wait,
	gps_header,
	gps_data,
	gps_finish
};


int Serialport::init(const char *devname, int mode, int speed, size_t size)
{
	if(mode == 0)
	{
		int myfd;
		struct stat myStatBuf;
		struct termio myTermIo;
		
		m_fd=open(devname, O_RDWR|O_NONBLOCK|O_NDELAY|O_EXCL|O_NOCTTY);
		//m_fd=open(devname, O_RDWR|O_NDELAY|O_EXCL|O_NOCTTY);
		//m_fd=open(devname, O_RDWR);
		if(m_fd < 0)
		{
			printf("open err %d\n", m_fd);
			return -1;
		}
		
		if (fstat(m_fd, &myStatBuf) != 0)
		{
			printf("err fstat\n");
			return -1;
		}
		if (!S_ISCHR(myStatBuf.st_mode))
		{
		      //Popup::Critical(QObject::tr("SECURITY ERROR: You are trying to open something that is not a character device\n" APPNAME " will close now."));
		      printf("SECURITY ERROR: You are trying to open something that is not a character device\n");
		      return -1;
		}
		
		if (ioctl (m_fd, TCGETA, &myTermIo) < 0) 
		{
		      printf("ioctl TCGETA\n");
		      return -1;
		}
	//	myTermIo.c_iflag &= ~(IGNCR | ICRNL | IGNBRK | IUCLC | INPCK | IXON | IXANY | IGNPAR );
	//	myTermIo.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONLCR | ONLRET);
	//	myTermIo.c_lflag &= ~(ICANON | XCASE | ECHO | ECHOE | ECHONL);
	//	myTermIo.c_lflag |= ISIG | IEXTEN | ECHOK | ECHOKE | ECHOCTL;
		myTermIo.c_iflag &= ~(IGNCR | ICRNL | IGNBRK | IUCLC | INPCK | IXON | IXANY | IGNPAR );
		myTermIo.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONLCR | ONLRET);
		myTermIo.c_lflag &= ~(ICANON | ISIG | XCASE | ECHO | ECHOE | ECHONL);
		myTermIo.c_lflag |= IEXTEN | ECHOK | ECHOKE | ECHOCTL;
		myTermIo.c_cc[VMIN] = 0;
		myTermIo.c_cc[VTIME] = 0;
		myTermIo.c_cc[VEOF] = 0;

		//modified by bobfsa 20140518
		 myTermIo.c_cflag &= ~(CBAUD | CSIZE | CSTOPB | CLOCAL | PARENB);
		//myTermIo.c_cflag &= ~(CBAUD | CSIZE | CLOCAL | PARENB);
		//myTermIo.c_cflag |= CSTOPB;
		
		myTermIo.c_cflag |= CLOCAL | HUPCL;
		myTermIo.c_cflag |= (speed | CS8 | CREAD );
		if (ioctl(m_fd, TCSETA, &myTermIo) < 0) 
		{
			printf("ioctl TCSETA\n");
		      return -1;
		}
		
		tcflush(m_fd,TCIFLUSH);
		usleep(200000L); /* Wait a bit (DTR raise) */	

	}
	else
	{
		struct stat myStatBuf;
		struct termios newtio, oldtio;
		speed_t myspeed;

		//m_fd=open(devname, O_RDWR|O_NONBLOCK|O_NDELAY|O_NOCTTY);
		m_fd=open(devname, O_RDWR|O_NONBLOCK|O_NDELAY|O_NOCTTY);
		if(m_fd < 0)
		{
			printf("open err %d\n", m_fd);
			return -1;
		}
		if (fstat(m_fd, &myStatBuf) != 0)
		{
			printf("err fstat\n");
			return -1;
		}
		if (!S_ISCHR(myStatBuf.st_mode))
		{
		      printf("SECURITY ERROR: You are trying to open something that is not a character device\n");
		      return -1;
		}	
		
	    	if  ( tcgetattr(m_fd,&oldtio)  !=  0)     
		{         
			perror("SetupSerial 2");        
			return -1;    
		}
		
	    	bzero( &newtio, sizeof( newtio ) );    
		newtio.c_cflag  |=  CLOCAL | CREAD;     
		newtio.c_cflag &= ~CSIZE; 
		newtio.c_cflag |= CS8;
		newtio.c_cflag &= ~PARENB;
		switch(speed)
		{
			case 9600:
			{
				myspeed=B9600;
				break;
			}
			case 115200:
			{
				myspeed=B115200;
				break;
			}
			default:
				break;
		}
	         cfsetispeed(&newtio, myspeed);        
		cfsetospeed(&newtio, myspeed);
		newtio.c_cflag &=  ~CSTOPB;
		newtio.c_cc[VTIME]  = 0;    
		newtio.c_cc[VMIN] = 0;
		tcflush(m_fd,TCIFLUSH);
				
	   	if((tcsetattr(m_fd,TCSANOW,&newtio))!=0)    
		{        
			printf("com set error");        
			return -1;    
		}

		usleep(200000L); /* Wait a bit (DTR raise) */	
	}
	
	m_brunning = 1;
	m_msgsize=size;

	start();	
	detach();	
	return 0;
}

void Serialport::set_boardctl(boardctl * ctl)
{
	m_board=ctl;
}

size_t Serialport::_read(char *buff, size_t size)
{
	return read(m_fd, buff, size);
}


size_t Serialport::_write(char *buff, size_t size)
{
	return write(m_fd, buff, size);
}

int Serialport::release()
{
	stop();
	m_brunning = 0;
	
	usleep(10000);
	if(m_fd)
		close (m_fd);
}

size_t Serialport::_read_block(char * buf, size_t size)
{
	int index=0;
	int nret=0;

	while(1)
	{
		nret=read(m_fd, &(buf[index]), size-index);
		if(nret > 0)
		{
			index+=nret;
			if(index>=size)
				break;
		}

		usleep(2000);
	}
	return index;
}


size_t Serialport::_read_msg(char * buf, size_t size)
{
	char *msg;
poll:
	if(! m_msgqueue.empty())
	{
		msg_mutex.lock();

		msg=m_msgqueue.front();
		memcpy(buf, msg, m_msgsize);
		m_msgqueue.pop();
		msg_mutex.unlock();

		delete msg;
	}
	else
	{
		usleep(5000);
		goto poll;
	}

	return m_msgsize;
}

static char timegps[BUF_LEN];
static char dategps[BUF_LEN];
static char locationgps[BUF_LEN];

void *Serialport::sub_routine(void)
{
#if defined(_THUNDER_)
	int len=0,start=0;
	int nret=0;
	static char buf[BUF_LEN*4];
	static gps_packet t_rxgpspkt;
	char *pmsg=NULL;	
	int tmp_time;

	int head_index=0,curr_index=0;
	int cnt=0;
	int gps_state=gps_wait;
	char *ptime=NULL;
	char *pdata=(char *)&t_rxgpspkt;


	printf("%s start : pkt size:%d\n",__func__, sizeof(gps_packet));
	while (m_brunning)	
	{		
		nret=read(m_fd, &buf[head_index], 1);
		if(nret <= 0)
		{
			usleep(10000);
			continue ;
		}
		//printf("rx: 0x%x\n", buf[head_index]);

		switch(gps_state)
		{
			case gps_wait:
			{
				if(buf[head_index]== '$')
				{
					*pdata++=buf[head_index];
					//printf("New Packet Start dsp_header\n");
					cnt = 1;
					head_index = 1;
					gps_state=gps_header;
				}
				break;
			}
			case gps_header:
			{
				*pdata++=buf[head_index];
				head_index++;
				if(head_index == strlen(locationGPS) &&\
					(memcmp(buf, locationGPS,  strlen(locationGPS)) == 0))
				{
					memcpy(pdata, locationGPS,  strlen(locationGPS));
					gps_state=gps_data;
					cnt=head_index;
					head_index=0;
					//printf(" Packet Start dsp_data: %d %d \n", head_index, strlen(result_msg_tag));
				}
				else if(head_index == strlen(locationGPS) &&\
					(memcmp(buf, locationGPS,  strlen(locationGPS))!= 0))
				{
					gps_state=gps_wait;
					head_index=0;
					pdata=(char *)&t_rxgpspkt;
					printf(" Packet back to  dsp_wait\n");
				}
				break;		
			}
			case gps_data:
			{
				*pdata++=buf[head_index];
				cnt ++;
				if(cnt == sizeof(gps_packet))
				{
					gps_state=gps_finish;
					cnt=0;	
					//printf(" Packet end dsp_finish\n");
				}
				break;
			}			
		}


		if(gps_state != gps_finish)
			continue ;

		gps_state=gps_wait;
		head_index=0;
		pdata=(char *)&t_rxgpspkt;		
		cnt=0;

		memset(buf, 0, sizeof(buf));
		memcpy(buf, t_rxgpspkt.header, 6);
		printf("header: %s\n", buf);
		
		memset(buf, 0, sizeof(buf));
		memcpy(buf, &(t_rxgpspkt.time[6]), 2);
		tmp_time=atoi(buf);
		tmp_time=(tmp_time+8)%24;
		sprintf(&buf[6], "%02d",tmp_time);
		memcpy(buf, &(t_rxgpspkt.time),6);
		memcpy(&buf[8], &(t_rxgpspkt.time[8]), 4);		
		printf("time: %s\n", buf);
		memcpy(t_rxgpspkt.time, buf, 12);
		
		memset(buf, 0, sizeof(buf));
		memcpy(buf, (t_rxgpspkt.availabe), 1);
		printf("availabe: %s\n", buf);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, t_rxgpspkt.longitude, 8);
		printf("longitude: %s\n", buf);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, t_rxgpspkt.latitude, 8);
		printf("latitude: %s\n", buf);
		memset(buf, 0, sizeof(buf));
		memcpy(buf, t_rxgpspkt.angle, 8);
		printf("angle: %s\n", buf);
		if(m_board)
		{
			m_board->submit_gpstime(t_rxgpspkt.time, 12);
			m_board->submit_location(t_rxgpspkt.longitude, 16);
			m_board->submit_valid_locate(t_rxgpspkt.availabe, 1);
			m_board->submit_angle(t_rxgpspkt.angle, 8);
		}
		
	}

#endif

#if defined(DSPGUN)
	int len=0,start=0;
	int nret=0;
	static char buf[BUF_LEN*4];
	char *pmsg=NULL;

	int head_index=0,curr_index=0;
	int cnt=0;
	int dsp_state=dsp_wait;
	char *ptime=NULL;


	static 	GunResult t_gunresult;
	char *pdata=(char *)&t_gunresult;
	extern char *result_msg_tag;

	//printf("%s start \n",__func__);
	while (m_brunning)	
	{		
		nret=read(m_fd, &buf[head_index], 1);
		if(nret <= 0)
		{
			usleep(10000);
			continue ;
		}
		//printf("rx: 0x%x\n", buf[head_index]);

		switch(dsp_state)
		{
			case dsp_wait:
			{
				if(buf[head_index]== '$')
				{
					*pdata++=buf[head_index];
					//printf("New Packet Start dsp_header\n");
					cnt = 1;
					head_index = 1;
					dsp_state=dsp_header;
				}
				break;
			}
			case dsp_header:
			{
				*pdata++=buf[head_index];
				head_index++;
				if(head_index == strlen(result_msg_tag) &&\
					(memcmp(buf, result_msg_tag,  strlen(result_msg_tag)) == 0))
				{
					memcpy(pdata, result_msg_tag,  strlen(result_msg_tag));
					dsp_state=dsp_data;
					cnt=head_index;
					head_index=0;
					//printf(" Packet Start dsp_data: %d %d \n", head_index, strlen(result_msg_tag));
				}
				else if(head_index == strlen(result_msg_tag) &&\
					(memcmp(buf, result_msg_tag,  strlen(result_msg_tag))!= 0))
				{
					dsp_state=dsp_wait;
					head_index=0;
					pdata=(char *)&t_gunresult;
					printf(" Packet back to  dsp_wait\n");
				}
				break;		
			}
			case dsp_data:
			{
				*pdata++=buf[head_index];
				cnt ++;
				if(cnt == sizeof(GunResult))
				{
					dsp_state=dsp_finish;
					cnt=0;	
					//printf(" Packet end dsp_finish\n");
				}
				break;
			}			
		}


		if(dsp_state != dsp_finish)
			continue ;
		else
		{
			dsp_state=dsp_wait;
			head_index=0;
			pdata=(char *)&t_gunresult;
		}
				

		head_index = 0;
		//t_gunresult.msg_type=ntohl(t_gunresult.msg_type);

		//printf("msg: tag 0x%x 0x%x 0x%x 0x%x  type:0x%x dis:0x%x mach:0x%x \n",\
		//		t_gunresult.msg_tag[0],t_gunresult.msg_tag[1],t_gunresult.msg_tag[2],t_gunresult.msg_tag[3],\
		//		t_gunresult.msg_type, t_gunresult.result.para.dis,t_gunresult.result.para.Mach_angle);
		
		if(t_gunresult.msg_type == gunresult_get)
		{
			printf("get result: dis: %f\n", t_gunresult.result.para.dis);
			printf("get result: Mach_angle: %f\n", t_gunresult.result.para.Mach_angle);
			printf("get result: vbullet: %f\n", t_gunresult.result.para.VBullet);
			printf("get result: DBullet: %f\n", t_gunresult.result.para.DBullet);
			printf("get result: LBullet: %f\n", t_gunresult.result.para.LBullet);
		}
		else 
		{
			printf("get nothing result\n");
		}
		
		cnt=0;
	}
#endif

	return NULL;
}


