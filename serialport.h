#ifndef __SERIAL_PORT_H__
#define __SERIAL_PORT_H__

#include "osal.h"
#include "thread.h"
#include "boardctl.h"


typedef struct 
{
	char header[6];
	char time[12];
	char availabe[1];
	char longitude[8];
	char latitude[8];
	char 	angle[8];
}__attribute__((packed)) gps_packet;

class Serialport:public Cthread
{
public:
	Serialport():m_fd(-1),speed(0),m_brunning(0){};
	~Serialport(){};

	int init(const char *name, int mode, int speed, size_t size);
	void set_boardctl(boardctl *ctl);
	size_t _read(char *buf, size_t size);
	size_t _write(char *buf, size_t size);
	size_t _read_block(char *buf, size_t size);		
	int release();

	void *sub_routine(void);
	size_t _read_msg(char * buf, size_t size);
	int	m_brunning;

private:
	char *devname;
	int speed;		
	int m_fd;
	int m_msgsize;
	std::queue<char *>	m_msgqueue;
	cmutex msg_mutex;
	boardctl *m_board;
};

#endif

