#ifndef _OSAL_H
#define _OSAL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <strings.h>
#include <assert.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <queue>
#include <vector>
#include <list>
#include <semaphore.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <arpa/inet.h>
 #include <netinet/in.h>
 #include <sys/wait.h>
 #include <netdb.h>
 #include <netinet/tcp.h>
#include <linux/byteorder/little_endian.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>
#include <execinfo.h>
#include <signal.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>

typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define BUF_LEN 0x200

extern void get_time(char *date, char *time);

#define ELEVEL						0
#define EDEBUG(level, format, ...)			{if (ELEVEL<level) printf("<%s><%s:%d>" format, __FILE__, __FUNCTION__, __LINE__, ## __VA_ARGS__);}

#endif


