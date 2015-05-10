#include "arbi.h"


arbi_socket::arbi_socket()
{
	int ret;
	
	m_rxbuf=evbuffer_new();
	if(!m_rxbuf)
		printf("%s new evbuffer failed\n", __func__);
	ret=evbuffer_enable_locking(m_rxbuf, NULL);
	printf("enable block: %d\n", ret);
}

arbi_socket::~arbi_socket()
{
}

void arbi_socket::init()
{
	m_brunning = 1;
	start();
	detach();	
}

void arbi_socket::submit(char  *tres, size_t len)
{
	evbuffer_add(m_rxbuf, tres, len);
}

void *arbi_socket::sub_routine(void)
{
	GunResult t_result;
	
	while(m_brunning)
	{
		if(evbuffer_get_length(m_rxbuf) == 0)
		{
			printf("%s \n", __func__);
			usleep(1000000);
			continue ;
		}

		evbuffer_remove(m_rxbuf,  &t_result, sizeof(GunResult));

		printf("get something result\n");
			
	}	
}


