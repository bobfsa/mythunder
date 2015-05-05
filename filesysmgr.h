/*
 * =====================================================================================
 *
 *       Filename:  filesysmgr.h
 *
 *    Description:  filesysmgr class
 *
 *        Version:  1.0
 *        Created:  08/08/2013 08:51:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  safang@iflytek.com 
 *        Company:  www.iflytek.com
 *
 * =====================================================================================
 */

#ifndef __FILE_SYSMGR_H
#define __FILE_SYSMGR_H

/*
 * =====================================================================================
 * system header
 * =====================================================================================
 */
// required for port speeds
#include <termio.h>
#include <string>
#include <list>

#include <stdio.h>
//////#include "linux/fs.h"

#include "criticalsection.h"
/*
 * =====================================================================================
 * class prototype
 * =====================================================================================
 */
/** The Cfilesys_mgr class sets up, maintains and reads/writes the serial port connection. 
 * 
 */

#define MAX_PATH 	128
#define DEFAULT_SYSMGR_FILE 	"filemgr.dat"
#define MAX_OPRATION_FD	16
#define MAX_CAPACITY_PERCENT 90
#define MAX_RESULTLEN 128

#define FILEHEADER_LEN 	0x400
#define FILENODE_LEN	0x200

#define FILESYSMGR_DEBUG_LVL 2

#define NODEINDEX_NULL -1

typedef struct 
{
	unsigned long device_capacity;
	unsigned long current_length;
	unsigned long nodenum;
	unsigned long nodeitem_len;
	int nodeindex_start, nodeindex_stop;	
	char reserved[FILEHEADER_LEN-24];
}filemgr_header;

typedef struct 
{
	unsigned long index;
	unsigned long datalen;
	unsigned long bitrate;
	char date[32];	
	char time[32];
	char data_file[MAX_PATH];
	unsigned long duration;
	char reserved[FILENODE_LEN-16-32-32-MAX_PATH];
}filemgr_node;

typedef struct 
{
	FILE *fp;
	time_t create;
	unsigned long length;
	char name[MAX_PATH];
	char date[32];
	char time[32];
}filemgr_handle;


class CFilesys_mgr
{
	public:
		CFilesys_mgr();
		CFilesys_mgr(const char *dev, const char *dir, unsigned long size);
		~CFilesys_mgr();

		unsigned long get_capacity();	
		CFilesys_mgr *get_instance();

		int submit(void *buff, size_t size);		
		
		int request_handle();

		filemgr_handle *new_record_handle();
		int release(filemgr_handle *handle);
		int remove_first();
		int append(filemgr_handle *datafh);
		int remove_content(FILE *fp, unsigned long total, unsigned long offset, unsigned long length);
		int report(int fd);	

	private:
		
		cmutex cs;		
		CFilesys_mgr *m_pfilesys;
		unsigned long _capacity;
		char _working_device[MAX_PATH];
		char _working_root_dir[MAX_PATH];
		bool _initialized;
		bool _already_existed;
		filemgr_header _mgrhdr;

		FILE *_fpsysmgr;

		filemgr_handle *m_handle;
		filemgr_node *m_node;
};
/*
 * =====================================================================================
 * end
 * =====================================================================================
 */
#endif // end of __FILE_SYSMGR_H

