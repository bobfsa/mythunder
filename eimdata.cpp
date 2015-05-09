#include "osal.h"
#include "thread.h"
#include "eimdata.h"


#define EIMUNIT_LEN 14400
static char readdata[EIMUNIT_LEN*2];


int EIMDATA::init(int devfd, CFilesys_mgr *fs, DataSocket *datasock)
{
	m_devfd=devfd;
	m_fssave=fs;
	m_sock=datasock;
	m_brunning = 1;
	if(!m_fssave)
		printf("EIM Data save file system not exist\n");
	
	start();
	detach();
}

int EIMDATA::init(int devfd, CFilesys_mgr * fs, boardctl *ctl)
{
	m_devfd=devfd;
	m_fssave=fs;
	m_sock=NULL;
	m_brunning = 1;
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

void *EIMDATA::sub_routine(void)
{
	size_t nbytes;
	
	while(m_brunning)			
	{				
		nbytes=read(m_devfd, readdata, EIMUNIT_LEN);		
		//if(nbytes>0 && nbytes<=EIMUNIT_LEN)				
		//	break;	
		//printf("eim rd: %d\n", nbytes);
		
		if(nbytes == 0)
		{
			usleep(10000);			
			continue ;
		}
		//else
		//	usleep(100000);
		
	
		if(m_fssave)
			m_fssave->submit(readdata, nbytes);

		//if(m_sock)
		//	m_sock->submit(readdata, nbytes);
		//if(m_outctl)
		//	m_outctl->submit(readdata, nbytes);
	}	

	printf("EIM %s exit\n", __func__);
}



