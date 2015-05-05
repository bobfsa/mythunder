#include "hidtemp.h"


int hid_device::hid_device_init(const char * dev)
{
	int fd;
	
	fd = open(dev,O_RDWR);
	if(fd == -1)
	{
		printf("open %s failure\n",dev);
		return -1;
	}

	m_devfd=fd;

       if (ioctl(m_devfd, HIDIOCGVERSION, &m_version) < 0)
            printf("HIDIOCGVERSION");
        else
        {
           // printf("HIDIOCGVERSION: %d.%d\n", (m_version>>16) & 0xFFFF, m_version & 0xFFFF);
            if (m_version != HID_VERSION)
                printf("WARNING: version does not match compile-time version\n");
        }
        if (ioctl(fd, HIDIOCGDEVINFO, &m_dinfo) < 0)
            printf("HIDIOCGDEVINFO");
        else
        {
            //printf("HIDIOCGDEVINFO: bustype=%d busnum=%d devnum=%d ifnum=%d\n" \
                "\tvendor=0x%04hx product=0x%04hx version=0x%04hx\n" \
                "\tnum_applications=%d\n", \
                m_dinfo.bustype, m_dinfo.busnum, m_dinfo.devnum, m_dinfo.ifnum, \
                m_dinfo.vendor, m_dinfo.product, m_dinfo.version, m_dinfo.num_applications);
	    printf("HIDIOCGDEVINFO: vendor=0x%04hx product=0x%04hx version=0x%04hx\n", m_dinfo.vendor, m_dinfo.product, m_dinfo.version);
        }
        
        if (ioctl(fd, HIDIOCGNAME(99), m_name) < 0)
            printf("HIDIOCGNAME failed");
        else
        {
            m_name[99] = 0;
            printf("HIDIOCGNAME: %s\n", m_name);
        }

	return m_devfd;
}

void hid_device::hid_device_close()
{
	if(m_devfd)
		close(m_devfd);
}

int hid_device::all_hid_device_report()
{
	struct hiddev_report_info rinfo;
	struct hiddev_field_info finfo;
	struct hiddev_usage_ref uref;
	int rtype, i, j;
	char *rtype_str;

	for (rtype = HID_REPORT_TYPE_MIN; rtype <= HID_REPORT_TYPE_MAX;	rtype++) 
	{
		#if 0
		switch (rtype) 
		{
			case HID_REPORT_TYPE_INPUT: rtype_str = "Input"; break;
			case HID_REPORT_TYPE_OUTPUT: rtype_str = "Output"; break;
			case HID_REPORT_TYPE_FEATURE: rtype_str = "Feature"; break;
			default: rtype_str = "Unknown"; break;
		}

		printf("Reports of type %s (%d):\n", rtype_str, rtype);
		#endif
		
		rinfo.report_type = rtype;
		rinfo.report_id = HID_REPORT_ID_FIRST;

		while (ioctl(m_devfd, HIDIOCGREPORTINFO, &rinfo) >= 0) 
		{
			printf(" Report id: %d (%d fields)\n",rinfo.report_id, rinfo.num_fields);
			for (i = 0; i < rinfo.num_fields; i++) 
			{ 
				memset(&finfo, 0, sizeof(finfo));
				finfo.report_type = rinfo.report_type;
				finfo.report_id = rinfo.report_id;
				finfo.field_index = i;
				ioctl(m_devfd, HIDIOCGFIELDINFO, &finfo);
				printf(" Field: %d: app: %04x phys %04x flags %x (%d usages) unit %x exp %d\n", \
						i, finfo.application, finfo.physical, finfo.flags,
						finfo.maxusage, finfo.unit, finfo.unit_exponent);

				memset(&uref, 0, sizeof(uref));

				for (j = 0; j < finfo.maxusage; j++)
				{
					uref.report_type = finfo.report_type;
					uref.report_id = finfo.report_id;
					uref.field_index = i;
					uref.usage_index = j;

					ioctl(m_devfd, HIDIOCGUCODE, &uref);
					ioctl(m_devfd, HIDIOCGUSAGE, &uref);

					printf(" Usage: %04x val %d\n", uref.usage_code, uref.value);
				}
			}

			rinfo.report_id |= HID_REPORT_ID_NEXT;
		}
	}

	printf("Waiting for events ... (interrupt to exit)\n");
}

int temperdev::init(const char * devname, boardctl *ctl)
{
	hid_device_init(devname);
	//all_hid_device_report();

	m_submitctl=ctl;
	m_brunning = 1;
	start();
	detach();	
}

int temperdev::release()
{
	stop();
	m_brunning = 0;
	
	usleep(10000);
	hid_device_close();
}


void *temperdev::sub_routine(void)
{
	struct hiddev_event ev[64];
	int index,nbytes;
	time_t curr_time;
	char name[100];
	char submit_temperatue[64];
	
	while(m_brunning)
	{
		nbytes = read(m_devfd, ev, sizeof(ev));

		if (nbytes < (int) sizeof(ev[0])) 
		{
			if (nbytes < 0)
				printf("\nevtest: error reading");

			return (void *)-1;
		}
		//printf("read event: 0x%x bytes\n", nbytes);

		//for (index = 0; index < nbytes / sizeof(ev[0]); index++) 
		//if(index==2 || index==3 || index==4)
		#if 0
		for(index=0;index<6;index++)
		{
			int idx = ev[index].hid;

			curr_time = time(NULL);

			strftime(name, sizeof(name), "%b %d %T", 	localtime(&curr_time));
			printf("%s: Event: usage %x , value %d\n", name, ev[index].hid, ev[index].value);	
			//printf("%s: Event: usage %x , value %d\n", name, ev[index].hid, ev[index].value);	
			curr_temperature=ev[index].value;
		}
		#endif
		//temperature_standard=ev[0].value*256*256*256+ev[1].value*256*256+ev[2].value*256+ev[3].value;
		
		temperature_standard=ev[2].value*256+ev[3].value;
		if(ev[1].value==0 && ev[2].value==0 && ev[3].value==0)
		{
		}
		else
		{
			curr_temperature=(float)temperature_standard*0.1;
			printf("curr temperature: %4.2f\r\n", curr_temperature);	
			sprintf(submit_temperatue, "%4.2f", curr_temperature);
		}
		usleep(400000);
		if(m_submitctl)
			m_submitctl->submit_tempature(submit_temperatue, 4);
	}

}


