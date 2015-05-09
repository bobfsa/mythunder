#ifndef __DATA_SOCKET_H__
#define __DATA_SOCKET_H__

#include "osal.h"
#include "thread.h"

typedef struct 
{
	char *buff;
	size_t length;
}dataitem;

typedef enum
{
	sock_uninit,
	sock_init_fail,
	sock_init_nodata,
	sock_dataing,
	sock_destroy
}socket_status;

class DataSocket:public Cthread
{
public:
	DataSocket():m_brunning(0),m_sockstatus(sock_uninit){};
	~DataSocket(){};

	int init(int sockfd, int port,size_t size);
	int init(const char *ipaddr, const char *port, struct event_base *m_evbase);
	int init(int sockfd, struct event_base *evbase);
	int restart();
	int setcb(bufferevent_data_cb cb1, bufferevent_data_cb cb2, bufferevent_event_cb cb3);
	int submit(char *buf, size_t size);
	int get_status();
	void release();

	void *sub_routine(void);
	int m_sockfd;

private:
	int m_port;
	int	m_brunning;
	int m_msgsize;
	std::queue<dataitem *>	m_msgqueue;
	cmutex msg_mutex;
	int m_sockstatus;

	char *m_destip;
	char *m_destport;
	
	struct event_base *m_evbase;
	struct bufferevent *m_bufev;

	bufferevent_data_cb readcb;
	bufferevent_data_cb writecb;
	bufferevent_event_cb eventcb;
};

#endif

