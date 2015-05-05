#ifndef __DSP_GUNPARSE_H
#define __DSP_GUNPARSE_H

#include "osal.h"
#include "serialport.h"

enum
{
	gunresult_get=1,
	gunresult_nothing=2
};


typedef struct {	
	float dis;
	float Mach_angle;
	float VBullet;
	float DBullet;
	float LBullet;
	float reserved;
} SGunShoot_Para;

typedef struct 
{
	unsigned char msg_tag[4];
	unsigned int msg_type;
	unsigned char reserved[16];
	union
	{
		SGunShoot_Para para;
		unsigned char nothing[sizeof(SGunShoot_Para)];
	}result;
}GunResult;

enum
{
	dsp_wait,
	dsp_header,
	dsp_data,
	dsp_finish
};

class gunparse:public Serialport
{
public:
	gunparse();
	~gunparse();

	//void *sub_routine(void);
	
public:
	GunResult last_result;
};


#endif

