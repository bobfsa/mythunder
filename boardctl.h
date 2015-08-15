#ifndef __EIMGPS_BOARDCTL_H
#define __EIMGPS_BOARDCTL_H

#include "osal.h"
#include "datasocket.h"

#define HOST_REQ_HDR 0x5A5A
#define TARGET_REQ_DATA 0x6C6D

#define DATA_PALOAD_LEN 1040000

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
	u8 valid_location[4];
	u8 gps_location[16];
	u8 yaw_angle[8];
	u8 temparture[4];
	u8 reserved[4];
	u8 capdata[DATA_PALOAD_LEN];
}__attribute__((packed))  targetreply_data;



class EIMDATA;

class boardctl:public Cthread
{
public:
	boardctl();
	~boardctl();

	void init(DataSocket *outsock);
	void set_data_sock(DataSocket *outsock);
	void set_data_interface(EIMDATA *inf);
	void submit(void *buf, size_t size);
	void 	submit_tempature(void * data, size_t len);
	void submit_gpstime(void * data, size_t len);
	void submit_location(void * data, size_t len);
	void submit_angle(void * data, size_t len);
	void submit_valid_locate(void * data, size_t len);
	void *sub_routine(void);
	void set_sys_time();
	void add_sys_second();
	void get_sys_time();
	
public:
	DataSocket *m_upsock;
	DataSocket *m_downsock;
	EIMDATA *m_eiminf;

	struct evbuffer *m_rxevbuf;
	
	int m_brunning;
	
public:
	u8 m_gpstime[12];
	u8 m_temparture[4];
	u8 m_location[16];
	u8 m_direction[4];
	u8 m_angle[8];
	u8 m_valid[4];
	u32 m_syssecond;
	cmutex timemutex;
};


void host_request_proc(boardctl *board,char  *data);


#endif


