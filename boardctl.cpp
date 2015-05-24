#include "osal.h"
#include "serialport.h"
#include "datasocket.h"
#include "hidtemp.h"
#include "filesysmgr.h"
#include "boardctl.h"
#include "eimdata.h"

#if 0
void thunderctl_proc(thunderctl_msg *pctlmsg, int m_sockfd)
{
#define MTU 1480
	thunderctl_msg *ackmsg;
	thunderdata_packet_header header;
	int length, index;
	
	u8 *p_actdata;
	u32 *datawrptr;
	
	if(pctlmsg->ctl_type != BOARD_DOWNLOAD_THUNDER_DATA)
	{
		printf("%s not BOARD_DOWNLOAD_THUNDER_DATA\n");
		return ;
	}

	ackmsg=(thunderctl_msg *)malloc(sizeof(thunderctl_msg));
	ackmsg->ctl_dir=DIREC_BOARD2PC;
	ackmsg->ctl_type=BOARD_DOWNLOAD_THUNDER_DATA;

	memset(ackmsg->ctl_op,0, 64);

	header.thunder_affair=1;
	header.thunder_ID=2;
	strcpy((char *)header.status_parameter,"0123456789");

	header.thunder_length=0x10000;

	length=sizeof(header)+header.thunder_length+sizeof(header.thunder_affair);

	p_actdata=(u8 *)malloc(length);

	memcpy(p_actdata, &header, sizeof(thunderdata_packet_header));
	datawrptr=(u32 *)(p_actdata+sizeof(thunderdata_packet_header));
	for(index=0;index<(header.thunder_length)/sizeof(int);index++)
		*datawrptr++=index;
	
	*datawrptr=0;

	index=0;
	while(index<length)
	{
		if(length-index>MTU)
			send(m_sockfd, p_actdata+index, MTU, MSG_NOSIGNAL);
		else
			send(m_sockfd, p_actdata+index, length-index, MSG_NOSIGNAL);
	}

	return ;
}

#endif

boardctl::boardctl()
{
	int ret;
	m_brunning=0;
	m_upsock=NULL;
	m_rxevbuf=evbuffer_new();
	if(!m_rxevbuf)
		printf("%s new evbuffer failed\n", __func__);
	ret=evbuffer_enable_locking(m_rxevbuf, NULL);
	printf("enable block: %d\n", ret);
}

boardctl::~boardctl()
{
	evbuffer_free(m_rxevbuf);
}

void boardctl::init(DataSocket * outsock)
{	
	m_brunning=1;
	m_upsock=outsock;
	start();
	detach();	
}

void boardctl::set_data_sock(DataSocket *outsock)
{
	m_upsock=outsock;
}

void  boardctl::set_data_interface(EIMDATA * inf)
{
	m_eiminf=inf;
}


void boardctl::submit(void *buf, size_t size)
{
	int ret=0;
	ret=evbuffer_add(m_rxevbuf, buf, size);
	if(ret != 0)
		printf("boardctl %s failed\n",__func__);
	
}

void boardctl::submit_tempature(void *data, size_t len)
{
	memcpy(m_temparture, data, len);
}

void boardctl::submit_gpstime(void *data, size_t len)
{
	memcpy(m_gpstime, data, len);
}

void boardctl::submit_location(void * data, size_t len)
{
	memcpy(m_location, data, len);
}
	
void boardctl::submit_angle(void * data, size_t len)
{
	memcpy(m_angle, data, len);
}
void boardctl::submit_valid_locate(void * data, size_t len)
{
	memcpy(m_valid, data, len);
}
	
void host_request_proc(boardctl *pboardctl, char  *data)
{
	hostreq_msg *msg=(hostreq_msg *)data;
	static targetreply_msg replymsg;
	
	replymsg.msg_hdr=TARGET_REQ_DATA;
	if(msg->msg_hdr != HOST_REQ_HDR)
	{
		printf("msg error\n");
		return ;
	}

	switch(msg->msg_type)
	{
		case msg_GPS:
		{
			replymsg.msg_type=msg_GPS;	
			memcpy(&(replymsg.body1), pboardctl->m_location, 8);				
			break;				
		}
		case msg_temperatue:
		{
			replymsg.msg_type=msg_temperatue;
			memcpy(&(replymsg.body1), pboardctl->m_temparture, 4);				
			break;			
		}
		case msg_direciton:
		{
			replymsg.msg_type=msg_direciton;
			memcpy(&(replymsg.body1), pboardctl->m_direction, 4);				
			break;		
		}
	}

	if(pboardctl->m_upsock)
		pboardctl->m_upsock->submit((char *)&replymsg, sizeof(targetreply_msg));	
}



void *boardctl::sub_routine(void)
{
	targetreply_data *repdata=new targetreply_data;
	static int  cnt=0;
	int pre_sock_NULL=1;

	//printf("board ctl %s start\n",__func__);
	while(m_brunning)
	{
		//printf("board ctl %s start\n", __func__);
		usleep(800000);

		if(evbuffer_get_length(m_rxevbuf) >= DATA_PALOAD_LEN)
		{
			evbuffer_remove( m_rxevbuf,  repdata->capdata, DATA_PALOAD_LEN);
		}
		else
		{
			//printf("m_rxevbuf len: %d\n", evbuffer_get_length(m_rxevbuf));
			 continue ;
		}

		if(m_upsock && pre_sock_NULL)
		{
			printf("board upsock ready!\n");
			m_eiminf->set_search_header();
			pre_sock_NULL=0;			
		}
		else if(!m_upsock)
		{
			//printf("board upsock not ready!\n");
			pre_sock_NULL=1;
		}

		repdata->msg_hdr=TARGET_REQ_DATA;
		repdata->msg_type=msg_datapacket;
		memcpy(repdata->temparture, m_temparture, sizeof(m_temparture));
		memcpy(repdata->gps_time, m_gpstime, sizeof(m_gpstime));
		memcpy(repdata->valid_location, m_valid, sizeof(m_valid));
		memcpy(repdata->gps_location, m_location, sizeof(m_location));
		memcpy(repdata->yaw_angle, m_angle, sizeof(m_angle));
		
		printf("can send one pkt: %d\r\n",  cnt);
		cnt++;
		if(m_upsock)
		{
			//printf("board ctl %s send one pkt\n", __func__);
			m_upsock->submit((char *)repdata, sizeof(targetreply_data));		
			//m_upsock->submit((char *)repdata->capdata, DATA_PALOAD_LEN);
		}
	}

	delete repdata;

	return NULL;
}


