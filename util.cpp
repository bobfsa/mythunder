#include "osal.h"

extern char timegps[];
extern char dategps[];

void get_time(char *date, char *time)
{
	static int x=20150405;
	static int y=212213;

	y++;
	sprintf(date, "%d",x);
	sprintf(time, "%d", y);
		
	//strcpy(date, dategps);
	//strcpy(time, timegps);

	return ;
}

int set_socket_param(int sockfd)
{
	int nNetTimeout=1000;	
	int keepalive = 1; 	
	int keepidle = 60; 	
	int keepinterval = 5;	
	int keepcount = 3;
	
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout,sizeof(int));
		
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));  
	setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));	
	setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));  
	setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));  
	return 0;
}
