#ifndef __EIMGPS_BOARDCTL_H
#define __EIMGPS_BOARDCTL_H

#include "osal.h"
#include "datasocket.h"

#define HOST_REQ_HDR 0x5A5A
#define TARGET_REQ_DATA 0x5A5B

#define DATA_PALOAD_LEN 960000

enum{
	msg_GPS=0x0110,
	msg_temperatue=0x0120,
	msg_direciton=0x0130,
	msg_datapacket=0x0020,
};

typedef struct
{
	u16 msg_hdr;
	u16 msg_type;
}hostreq_msg;

typedef struct
{
	u16 msg_hdr;
	u16 msg_type;
	u32 body1;
	u32 body2;
}targetreply_msg;

typedef struct 
{
	u16 msg_hdr;
	u16 msg_type;
	u8 gps_time[12];
	u8 temparture[4];
	u8 capdata[DATA_PALOAD_LEN];
}targetreply_data;




class boardctl:public Cthread
{
public:
	boardctl();
	~boardctl();

	void init(DataSocket *outsock);
	void submit(void *buf, size_t size);
	void 	submit_tempature(void * data, size_t len);
	void submit_gps(void * data, size_t len);
	void submit_location(void * data, size_t len);
	void *sub_routine(void);


public:
	DataSocket *m_upsock;
	DataSocket *m_downsock;

	struct evbuffer *m_rxevbuf;
	
	int m_brunning;
	
public:
	u8 m_gpstime[12];
	u8 m_temparture[4];
	u8 m_location[16];
	u8 m_direction[4];
};


void host_request_proc(boardctl *board,char  *data);


#endif


