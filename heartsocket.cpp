#include "heartsocket.h"

int HeartSocket::init(int sockfd,  struct event_base *evbase)
{
        m_bufev = bufferevent_socket_new(evbase, sockfd, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);  
  	m_evbase=evbase;

	printf("bufferevent_socket_new OK\n");
	m_sockstatus = heartsock_init;
	m_brunning = 1;
	m_sockfd=sockfd;
	start();
	detach();	
}

int HeartSocket::setcb(bufferevent_data_cb cb1, bufferevent_data_cb cb2, bufferevent_event_cb cb3)
{
	if(m_bufev)
	{
		bufferevent_setcb(m_bufev, cb1, cb2, cb3, this);
		bufferevent_enable(m_bufev, EV_READ|EV_WRITE|EV_PERSIST);
	}
}

