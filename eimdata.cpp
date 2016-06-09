#include "osal.h"
#include "thread.h"
#include "eimdata.h"




static char readdata[EIMUNIT_LEN*2];


int EIMDATA::init(int devfd, CFilesys_mgr *fs, DataSocket *datasock)
{
	m_devfd=devfd;
	m_fssave=fs;
	m_sock=datasock;
	m_brunning = 1;
	m_first_searched=0;
	if(!m_fssave)
		printf("EIM Data save file system not exist\n");
	
	start();
	detach();
}

int EIMDATA::init(int devfd, CFilesys_mgr * fs, boardctl *ctl)
{
	m_devfd=devfd;
	m_fssave=fs;
	m_outctl=ctl;
	m_sock=NULL;
	m_brunning = 1;
	m_first_searched=0;
	if(!m_fssave)
		printf("EIM Data save file system not exist\n");
	
	start();
	detach();	
}

void EIMDATA::release()
{
	stop();
	m_brunning = 0;
	usleep(1000000);		
}

void EIMDATA::set_search_header()
{
	search_mutex.lock();
	m_first_searched=0;
	search_mutex.unlock();
}

void *EIMDATA::sub_routine(void)
{
	size_t nbytes;
	int first_frame=0;
	short *pdata;
	int index=0;
	u32 idle_cnt=0;
	int pre_sock_NULL=1;	

	while(m_brunning)			
	{				
		nbytes=read(m_devfd, readdata, EIMUNIT_LEN);		
		//if(nbytes>0 && nbytes<=EIMUNIT_LEN)				
		//	break;	
		
		if(nbytes == 0)
		{
			usleep(500);			
			continue ;
		}

		idle_cnt = 0;
		index=0;
		
	
		if(m_outctl)
			m_outctl->submit(readdata, nbytes);
	}	

	//printf("EIM %s exit\n", __func__);
}



