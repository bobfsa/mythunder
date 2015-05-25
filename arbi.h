#ifndef __EIM_ARBISOCKET_H
#define __EIM_ARBISOCKET_H

#include "osal.h"
#include "datasocket.h"
#include "gunparse.h"


class arbi_socket:public DataSocket
{
public:
	arbi_socket();
	~arbi_socket();

	void init();
	void submit(char *data, size_t len);
	void *sub_routine(void);
	void update_client_num(int add);
	
private:
	std::vector<DataSocket *> m_arbivec;
	struct evbuffer *m_rxbuf;
	cmutex vecmutex;
	int m_client_num;
};




#endif


