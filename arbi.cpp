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

void arbi_socket::update_client_num(int add)
{
	vecmutex.lock();
	if(add)
		m_client_num++;
	else
		m_client_num--;
	vecmutex.unlock();
}

void arbi_socket::submit(char  *tres, size_t len)
{
	evbuffer_add(m_rxbuf, tres, len);
}

void *arbi_socket::sub_routine(void)
{
	GunResult t_result;
	int index=0;
	time_t first_rxtime={0},curr_rxtime;
	int newresult=1;
	static std::vector<GunResult> reslist;
	int rxnum=0;
	
	while(m_brunning)
	{
		if(evbuffer_get_length(m_rxbuf) == 0)
		{
			usleep(1000000);
			continue ;
		}

		time(&first_rxtime);
		while(newresult) // rx new gunresult packet
		{
			if(evbuffer_get_length(m_rxbuf) == 0)
			{
				printf("%s \n", __func__);
				usleep(10000);
				continue ;
			}

			evbuffer_remove(m_rxbuf,  &t_result, sizeof(GunResult));
			reslist.push_back(t_result);
			if(reslist.size() >= m_client_num)
				break;

			time(&curr_rxtime);
			if(curr_rxtime-first_rxtime >= 5)
				break;

		}

		newresult = 0;
		rxnum=reslist.size();
		std::vector<GunResult>::iterator myiter;
		for(myiter=reslist.begin();myiter!=reslist.end();myiter++)
		{
			//arbi argorthim
			//printf("re:0x%x\n", *myiter.gxxx);
		}

		newresult = 1;
		printf("get something result\n");
			
	}	
}


