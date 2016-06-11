#ifndef __HEARTBEAT_SOCKET_H__
#define __HEARTBEAT_SOCKET_H__

#include "osal.h"
#include "thread.h"

enum
{
	heartsock_init,
	heartsock_beat,
	heartsock_timeout,
};

class HeartSocket:public Cthread
{
public:
	HeartSocket():m_brunning(0){};
	~HeartSocket(){};

	int init(int sockfd, struct event_base *evbase);
	int setcb(bufferevent_data_cb cb1, bufferevent_data_cb cb2, bufferevent_event_cb cb3);
	void release();

	void *sub_routine(void);

private:
	int m_port;
	int m_brunning;
	int m_msgsize;
	int m_sockstatus;

	char *m_destip;
	char *m_destport;
	
	struct event_base *m_evbase;
	struct bufferevent *m_bufev;
	struct event evtimer;
};

#endif


