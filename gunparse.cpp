#include "gunparse.h"


char *result_msg_tag="$FSA";

#if 0
void *gunparse::sub_routine(void)
{
	int len=0,start=0;
	int nret=0;
	static char buf[BUF_LEN*4];
	static char gpsdata[BUF_LEN];	

	char *pmsg=NULL;

	int head_index=0,curr_index=0;
	int cnt=0;
	int gps_state=gps_wait;
	char *ptime=NULL;

	//printf("%s start \n",__func__);
	while (m_brunning)	
	{		
		nret=read(m_fd, &(buf[head_index]), 1);
		if(nret <= 0)
		{
			usleep(10000);
			continue ;
		}
		printf("rx: 0x%x\n", buf[0]);

		{		
			switch(gps_state)
			{
				case gps_wait:
				{
					if(buf[head_index]== '$')
					{
						//printf("New Packet Start\n");
						cnt = nret;
						head_index = nret;
						//cnt=1;
						gps_state=gps_data;
					}
					break;
				}
				case gps_header:
				{
					break;		
				}
				case gps_data:
				{
					cnt += nret;
					head_index += nret;
					//cnt++;					
					if(buf[cnt-2]==0xd && buf[cnt-1]==0xa)
					{
						gps_state=gps_finish;
					}
					break;
				}
			}

		}

		if(gps_state != gps_finish)
			continue ;
		else
		{
			gps_state=gps_wait;
			head_index=0;
		}
				

		memset(gpsdata, 0, sizeof(gpsdata));
		memcpy(gpsdata, buf, cnt);
		head_index = 0;


		if(strncmp((const char *)gpsdata, tgtGPS, strlen(tgtGPS)) != 0)
		{			
		}
		else
		{
			memset(timegps, 0, sizeof(timegps));
			ptime=strchr(gpsdata, ',')+1;
			memcpy(timegps, ptime, strchr(ptime, ',')-ptime);
			printf("Time GPS: %s\n", timegps);

			if(m_board)
				m_board->submit_gps(timegps, 12);

			for(int i=0;i<8;i++)
				ptime=strchr(ptime,',')+1;

			memset(dategps, 0, sizeof(dategps));
			memcpy(dategps, ptime, strchr(ptime, ',')-ptime);
			printf("Date GPS: %s\n", dategps);

			if(m_board)
				m_board->submit_location(dategps, 12);
		}		

		cnt=0;


	}

	return NULL;
}
#endif

