#ifndef __EIMDATA_H__
#define __EIMDATA_H__

#include "osal.h"
#include "thread.h"
#include "datasocket.h"
#include "boardctl.h"
#include "filesysmgr.h"

class EIMDATA:public Cthread
{
public:
	EIMDATA():m_brunning(0),sockfd(-1),m_devfd(-1){};
	~EIMDATA(){};

	//int init(int sockfd, int port,size_t size);
	//size_t _write(char *buf, size_t size);
	int init(int devfd, CFilesys_mgr *fs, DataSocket *sendsock);
	int init(int devfd,  CFilesys_mgr *fs, boardctl *ctl);
	int init(int devfd, int sockfd, CFilesys_mgr *fs);
	int add_outfd(int fd);
	int get_status();
	void release();

	void *sub_routine(void);

private:
	
	int m_devfd;
	int m_brunning;
	cmutex msg_mutex;

	CFilesys_mgr *m_fssave;
	DataSocket *m_sock;
	boardctl *m_outctl;
	int sockfd;
};



#endif
