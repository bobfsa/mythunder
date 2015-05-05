#ifndef __HID_TEMPE_H
#define __HID_TEMPE_H

#include "osal.h"
#include "thread.h"
#include "boardctl.h"

class hid_device
{
public:
	hid_device(){};
	~hid_device(){};

	int hid_device_init(const char *dev);
	void hid_device_close();
	int all_hid_device_report();

public:
	int m_devfd;
 	struct hiddev_devinfo m_dinfo;
	int m_version;
	char m_name[100];
};


class temperdev:public hid_device,public Cthread
{
public:
	temperdev(){};
	~temperdev(){};

	int init(const char *devname, boardctl *submit);
	int release();

	void *sub_routine(void);
private:
	int m_brunning;
	int temperature_standard;
	float curr_temperature;
	boardctl *m_submitctl;
};

#endif
