#include "osal.h"
#include "thread.h"
#include "eimdata.h"


#define EIMUNIT_LEN 15600
#define EIMUNIT_HEADER 	0x5A5B
#define EIMUNIT_HDRLEN	0x0034

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
	m_outctl=ctl;
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
	int first_frame=0;
	short *pdata;
	int index=0;
	u32 idle_cnt=0;
	u32 test_cnt=0;
	
	while(m_brunning)			
	{				
		nbytes=read(m_devfd, readdata, EIMUNIT_LEN);		
		//if(nbytes>0 && nbytes<=EIMUNIT_LEN)				
		//	break;	
		//printf("eim rd: %d\n", nbytes);
		
		if(nbytes == 0)
		{
			usleep(5000);			
			idle_cnt++;
			if(idle_cnt > 800)
			{
				write(m_devfd, readdata, 1);
				idle_cnt = 0;
			}
			continue ;
		}

		idle_cnt = 0;
		test_cnt++;
		if(test_cnt >= 0x10000)
		{
			write(m_devfd, readdata, 1);
			test_cnt=0;
		}
		index=0;
		if(first_frame == 0)
		{
			pdata=(short *)readdata;
			index=0;
			do
			{
				if(*pdata == EIMUNIT_HEADER && *(pdata+1)==EIMUNIT_HDRLEN)
				{		
					first_frame=1;
					break;
				}
				pdata++;
				index++;
			}while((index*2)<nbytes);
		}


		
	
		//if(m_fssave)
		//	m_fssave->submit(readdata, nbytes);
		//if(m_sock)
		//	m_sock->submit(readdata, nbytes);
		if(m_outctl)
			m_outctl->submit(readdata, nbytes-(index*2));
	}	

	//printf("EIM %s exit\n", __func__);
}



