/*
 * =====================================================================================
 *
 *       Filename:  serialport.cpp
 *
 *    Description:  serialport class operation definition
 *
 *        Version:  1.0
 *        Created:  01/17/2011 08:51:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  yongwu@iflytek.com 
 *        Company:  www.iflytek.com
 *
 * =====================================================================================
 */

/*
 * =====================================================================================
 * system header
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <time.h>
#include <sys/mman.h>
#include<sys/stat.h>
#include <errno.h>
#include "time.h"
/*
 * =====================================================================================
 * local header
 * =====================================================================================
 */
#include "filesysmgr.h"
#include "osal.h"


#define DATA_REC_UNITLEN	0x1000000

#define TEST_NFS

CFilesys_mgr::CFilesys_mgr(const char * dev, const char * dir, unsigned long dev_size)
{
	char sysfile[MAX_PATH];
	int devfd=0;
	FILE *fp_sys=NULL;
	filemgr_node *pnode=new filemgr_node;
	int index=0;

	_initialized=0;
	m_pfilesys=NULL;

#ifndef TEST_NFS
	if((devfd=open(dev,O_EXCL|O_RDWR) ) == 0)
	{
		EDEBUG(0,"device open failed\n");
		return ;
	}	
#endif

	EDEBUG(FILESYSMGR_DEBUG_LVL,"OPEN DEVICE OK\n");

	sprintf(sysfile,"%s/"DEFAULT_SYSMGR_FILE,dir);
	if((fp_sys=fopen(sysfile,"rb")) == NULL)
	{
		EDEBUG(0,"sysmgr file open failed\n");
		_already_existed=0;
	}
	else
	{
		fclose(fp_sys);
		_already_existed=1;
	}
	
	EDEBUG(FILESYSMGR_DEBUG_LVL,"open file %s== %d\n", sysfile, _already_existed);


	strcpy(_working_device,dev);
	strcpy(_working_root_dir, dir);

	if(_already_existed)
	{
		fp_sys=fopen(sysfile,"r+b");		
		
		fseek(fp_sys, 0, SEEK_SET);
		fread(&_mgrhdr, 1, sizeof(filemgr_header), fp_sys);	
		fseek(fp_sys, sizeof(filemgr_header), SEEK_SET);
		_capacity=_mgrhdr.device_capacity;
		EDEBUG(FILESYSMGR_DEBUG_LVL,"Filesys cap:0x%x currlen:0x%x recNum:0x%x nodelen:0x%x-0x%x\n",\
			_mgrhdr.device_capacity, _mgrhdr.current_length, _mgrhdr.nodenum,\
			_mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);
		_fpsysmgr=fp_sys;
		
		fseek(fp_sys, sizeof(filemgr_header), SEEK_SET);
		for(index=0;index<_mgrhdr.nodenum;index++)
		{
			fread(pnode, 1, sizeof(filemgr_node), fp_sys);
			EDEBUG(FILESYSMGR_DEBUG_LVL,"Nodeindex:0x%x currlen:0x%x File:%s\n",\
				pnode->index, pnode->datalen, pnode->data_file);		
		}
	}
	else
	{
		int min_sector_size;
		int sectors;
	
		if((fp_sys=fopen(sysfile,"w+b")) == NULL)
		{
			EDEBUG(0,"sysmgr file create failed\n");
			return ;
		}
		
#ifndef TEST_NFS
		ioctl(devfd, BLKSSZGET, &min_sector_size);
		ioctl(devfd, BLKGETSIZE, &sectors);
		EDEBUG(FILESYSMGR_DEBUG_LVL,"get device: sec_sz:0x%x secs:0x%x\n", min_sector_size, sectors);
#else
		min_sector_size=1;
		sectors=1024*1024*256;

#endif
		_capacity=min_sector_size*sectors;

		if(dev_size < _capacity)
		{
			_capacity=dev_size;
		}

		memset(&_mgrhdr, 0, sizeof(filemgr_header));
		_mgrhdr.device_capacity=_capacity;
		_mgrhdr.nodeitem_len=sizeof(filemgr_node);
		_mgrhdr.nodeindex_start=NODEINDEX_NULL;
		_mgrhdr.nodeindex_stop=NODEINDEX_NULL;
		
		fwrite(&_mgrhdr, 1, sizeof(filemgr_header), fp_sys);
		fflush(fp_sys);
		fseek(fp_sys, 0, SEEK_SET);	

		EDEBUG(FILESYSMGR_DEBUG_LVL,"Filesys cap:0x%x currlen:0x%x recNum:0x%x nodelen:0x%x-0x%x\n",\
			_mgrhdr.device_capacity, _mgrhdr.current_length, _mgrhdr.nodenum,\
			_mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);		

		_fpsysmgr=fp_sys;		
	}	

	m_pfilesys=this;
	m_handle=NULL;

	return ;
}


CFilesys_mgr::~CFilesys_mgr()
{
	fclose(_fpsysmgr);
}

CFilesys_mgr *CFilesys_mgr::get_instance()
{
	return m_pfilesys;
}

int CFilesys_mgr::request_handle()
{
#if 0
	int indexdata=0,indextext=0, start=0;		
	//FILE *fd, *tmpfd;
	filemgr_handle *fh, *tmptextfh, *tmpdatafh;
	Ccritical_scope mycs(&cs);
	char filename[MAX_PATH];	
	int ret=0;
	struct tm *now;
	time_t totaltime;

	if(m_pfilesys == NULL)
		return -1;

	for(indexdata=0;indexdata<MAX_OPRATION_FD;indexdata++)
	{
		if(_datafd[indexdata] == NULL)
			break;
	}
	if(indexdata>= MAX_OPRATION_FD)
	{
		return -1;
	}
	
	for(indextext=0;indextext<MAX_OPRATION_FD;indextext++)
	{
		if(_textfd[indextext] == NULL)
			break;
	}
	if(indextext>= MAX_OPRATION_FD)
	{
		return -1;
	}
	

	time(&totaltime);
	now=localtime(&totaltime);
	//if(fdtype == DATA_FD)
	{
		char tmp[MAX_PATH];
		
		tmpdatafh=new filemgr_handle;	
		memset(tmpdatafh, 0 ,sizeof(filemgr_handle));
		tmpdatafh->fp_map=NULL;
		strftime(tmp,MAX_PATH, "pcm_%Y%m%d%H%M%S.pcm",now);

		sprintf(filename,"%s%s", _working_root_dir, tmp);
		if((tmpdatafh->fp=fopen(filename, "w+b")) ==NULL)
		{
			EDEBUG(0, "create file %s failed\n", filename);
			delete tmpdatafh;
			return -1;
		}

		_datafd[indexdata]=tmpdatafh;
		_datafd[indexdata]->create=totaltime;
		strcpy(_datafd[indexdata]->name, filename);
		datafd=indexdata;
	}
	//else
	{
		tmptextfh=new filemgr_handle;		
		memset(tmptextfh, 0, sizeof(filemgr_handle));
		tmptextfh->create=totaltime;
		tmptextfh->fp_map=NULL;
		sprintf(filename, "%s%s", _working_root_dir, DEFAULT_TEXT_FILE);
		if((tmptextfh->fp=fopen(filename, "r+b")) ==NULL)
		{
			if((tmptextfh->fp=fopen(filename,"w+b")) == NULL)
			{
				EDEBUG(0, "open file %s failed\n", filename);
				datafd=-1;
				_datafd[indexdata]=NULL;
				delete tmpdatafh;
				delete tmptextfh;			
				return -1;
			}
		}

		_textfd[indextext]=tmptextfh;	
		_textfd[indextext]->create=totaltime;
		strcpy(_textfd[indextext]->name, filename);
		textfd=(indextext+MAX_OPRATION_FD);
		fseek(tmptextfh->fp, _mgrhdr.textlen, SEEK_SET);
	}	
#endif	
	return 1;	
}

filemgr_handle *CFilesys_mgr::new_record_handle()
{
	char date_str[MAX_PATH];
	char time_str[MAX_PATH];
	unsigned long pos;
	size_t nbytes;
	
	m_handle = new filemgr_handle;
	m_node = new filemgr_node;
	
	memset(m_node, 0, sizeof(filemgr_node));
	pos=sizeof(filemgr_header)+_mgrhdr.nodenum*sizeof(filemgr_node);
	
	fseek(_fpsysmgr, pos, SEEK_SET);	
	if(_mgrhdr.nodenum == 0)
	{
		m_node->index=0;
	}
	else	
		m_node->index=_mgrhdr.nodeindex_stop+1;	
	
	
get_new_time:
	get_time(date_str, time_str);

	sprintf(m_handle->name,"data_%s_%s.dat",date_str, time_str);
	strcpy(m_handle->date, date_str);
	strcpy(m_handle->time, time_str);

	m_handle->fp=fopen(m_handle->name, "rb");//try to open file
	if(m_handle->fp != NULL)
	{
		printf("file %s already exist!!\n",m_handle->name);
		fclose(m_handle->fp);
		goto get_new_time;
	}	
	

	
	m_handle->fp=fopen(m_handle->name,"wb");
	if(m_handle->fp)
	{
		printf("create file %s ok\n", m_handle->name);
	}
	else
	{
		printf("create file %s failed\n", m_handle->name);
		return NULL;
	}

	printf("New file:%s date:%s time:%s\n", m_handle->name, date_str, time_str);
	m_handle->length=0;
	time(&(m_handle->create));	
	

	m_node->datalen=m_handle->length;	
	strcpy(m_node->date, m_handle->date);
	strcpy(m_node->time, m_handle->time);
	strcpy(m_node->data_file, m_handle->name);	

	EDEBUG(FILESYSMGR_DEBUG_LVL,"new node: index:0x%x  datlen:0x%x\n", m_node->index,\
		 m_node->datalen);
	
	fseek(_fpsysmgr, pos, SEEK_SET);	
	nbytes=fwrite(m_node, 1, sizeof(filemgr_node), _fpsysmgr);

	if(_mgrhdr.nodenum == 0)
	{
		_mgrhdr.nodeindex_start=_mgrhdr.nodeindex_stop=0;
		_mgrhdr.nodenum=1;
	}
	else
	{
		_mgrhdr.nodeindex_stop+=1;
		_mgrhdr.nodenum++;
	}
	fseek(_fpsysmgr, 0, SEEK_SET); 
	fwrite(&_mgrhdr, 1, sizeof(filemgr_header), _fpsysmgr);	
	fflush(_fpsysmgr);	
	
	EDEBUG(FILESYSMGR_DEBUG_LVL, "Now FileSys Len:0x%x nodenum:0x%x from 0x%x to 0x%x\n",\
		_mgrhdr.current_length,_mgrhdr.nodenum, _mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);	
	
	return m_handle;
}

#if 0
void CFilesys_mgr::new_record_node(filemgr_handle *phandle)
{
	filemgr_node mynode;

	mynode.index=_mgrhdr.nodeindex_stop+1;
	mynode.datalen=phandle->length;
	
	strcpy(mynode.date, phandle->date);
	strcpy(mynode.time, phandle->time);
	strcpy(mynode.data_file, phandle->name);	

	fseek(_fpsysmgr, 0, SEEK_END);
	fwrite(&mynode, 1, sizeof(filemgr_node), _fpsysmgr);
	fflush(_fpsysmgr);

	_mgrhdr.nodeindex_stop+=1;

	printf("New Data Node:%s date:%s time:%s\n", \
		mynode.data_file, mynode.date, mynode.time);
	
	return ;
}
#endif


int CFilesys_mgr::submit(void *buff, size_t size)
{
	int nbytes;
	time_t totaltime;
	struct tm *now;
	mutex_scope mycs(&cs);
	
	if(m_pfilesys == NULL)
		return -1;
	
	//printf("CFilesys_mgr %s %d \n", __func__, size);

	if(m_handle== NULL)
	{
		printf("new_record_handle \n");
		m_handle=new_record_handle();
	}

	nbytes=fwrite(buff, 1, size,m_handle->fp);
	m_handle->length+=size;

	if(m_handle->length >= DATA_REC_UNITLEN)
	{
		fclose(m_handle->fp);

		//new_record_node(m_handle);
		release(m_handle);
		
		delete m_handle;
		m_handle=NULL;
		delete m_node;
		m_node=NULL;
	}
	return nbytes;	
}


int CFilesys_mgr::release(filemgr_handle *fh)
{
	int remove=0;
	mutex_scope mycs(&cs);
	int ret=0;
	unsigned long long temp=0;

	if(m_pfilesys == NULL)
		return -1;

	printf("file sys len: 0x%x device_capacity:0x%x\n", _mgrhdr.current_length, _mgrhdr.device_capacity);
	temp=((unsigned long long)_mgrhdr.current_length*100);
	if( _mgrhdr.current_length > _mgrhdr.device_capacity)
	{
		EDEBUG(2,"Too many files Len:0x%x need REMOVE!!\n", _mgrhdr.current_length);
		remove=1;
	}	

	if(remove)
	{
		remove_first();
	}
	
	if((ret=append(fh))>0)
	{
		EDEBUG(FILESYSMGR_DEBUG_LVL,"append success\n");
	}
	else
	{
		EDEBUG(2,"append failed\n");
	}

	//fclose(fh->fp);	
	return 0;
}

int CFilesys_mgr::remove_content(FILE *fp, unsigned long total, unsigned long offset, unsigned long rmlen)
{
	const int MAX_COPYBYTES=rmlen;
	int nbytes_read=MAX_COPYBYTES,nbytes_write;
	int offset_read, offset_write;
	char *transit=new char[MAX_COPYBYTES];
	int fd;
	struct stat statbuf;
	char *mapaddr=NULL;

	if(!fp || !rmlen)
		return -1;
	


#if 0
	fseek(fp, 0, SEEK_END);
	total=ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if((fd=fileno(fp)) == -1)
	{		
		EDEBUG(2, "fp:0x%x fileno failed err:0x%x \n",fp, errno);
	}
	if(fstat(fd,  &statbuf) != 0)
	{
		EDEBUG(2, "fd:0x%x fstat failed 0x%x  err:0x%x \n",fd, errno);
	}	
	total=statbuf.st_size;
	EDEBUG(FILESYSMGR_DEBUG_LVL, "fd:0x%x st_size:0x%x  \n",fd, total);
	
	mapaddr=(char *)mmap(NULL,total, (PROT_READ|PROT_WRITE), MAP_PRIVATE, fd, 0);
	if(mapaddr == MAP_FAILED)
	{
		EDEBUG(2, "fd:0x%x mmap failed 0x%x  err:0x%x \n",fd, total, errno);
		return -1;
	}
	else
	{
		EDEBUG(FILESYSMGR_DEBUG_LVL, "mapaddr:0x%x \n", mapaddr);
	}

	memmove(mapaddr+offset, mapaddr+offset+rmlen, total-offset-rmlen);
	memset(mapaddr+total-rmlen, 0, rmlen);
	munmap(mapaddr, total);
#endif

	if(total == 0)
	{
		fseek(fp, 0, SEEK_END);
		total=ftell(fp);
	}

	offset_read=offset+rmlen;
	offset_write=offset;	
	nbytes_read=MAX_COPYBYTES;
	do
	{
		fseek(fp, offset_read, SEEK_SET);
		nbytes_read=fread(transit, 1, MAX_COPYBYTES, fp);
		fseek(fp, offset_write, SEEK_SET);
		nbytes_write=fwrite(transit,1, nbytes_read,fp);
		
		offset_read+=nbytes_read;
		offset_write+=nbytes_write;
	}while(nbytes_read==MAX_COPYBYTES);

	fseek(fp, total-rmlen, SEEK_SET);
	memset(transit, 0, rmlen);
	fwrite(transit, 1, rmlen, fp);
	
	return rmlen;
}

int CFilesys_mgr::remove_first()
{
	filemgr_node recnode;
	filemgr_handle *fh;
	FILE *tmpfd;
	int nodelen=sizeof(filemgr_node);
	char cmd[MAX_PATH];

	if(m_pfilesys == NULL)
		return -1;

	fseek(_fpsysmgr, sizeof(filemgr_header), SEEK_SET);
	fread(&recnode, 1, nodelen, _fpsysmgr);
	
	EDEBUG(FILESYSMGR_DEBUG_LVL, "Remove node index:0x%x datalen:0x%x	audio file:%s\n",\
		recnode.index, recnode.datalen, recnode.data_file);
	if(remove_content(_fpsysmgr, 0, nodelen, nodelen) == nodelen)
	{
		EDEBUG(FILESYSMGR_DEBUG_LVL, "Remove _fpsysmgr remove_content 0x%x  OK\n",nodelen);
	}
	else
	{
		EDEBUG(FILESYSMGR_DEBUG_LVL, "Remove _fpsysmgr remove_content 0x%x  failed\n",nodelen);		
		return -1;
	}
	sprintf(cmd, "rm -f %s", recnode.data_file);
	system(cmd);	
	EDEBUG(FILESYSMGR_DEBUG_LVL, "Remove data file:%s\n",recnode.data_file);
	
	_mgrhdr.nodenum--;
	_mgrhdr.nodeindex_start++;
	_mgrhdr.current_length-=recnode.datalen;

	fseek(_fpsysmgr, 0, SEEK_SET);
	fwrite(&_mgrhdr, 1, sizeof(filemgr_header), _fpsysmgr);
	fflush(_fpsysmgr);	
	
	EDEBUG(FILESYSMGR_DEBUG_LVL, "Now FileSys Len:0x%x nodenum:0x%x from 0x%x to 0x%x\n",\
		_mgrhdr.current_length,_mgrhdr.nodenum, _mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);

	return nodelen;
}


int CFilesys_mgr::append(filemgr_handle *datafh)
{	
	int nbytes;
	int firstnode=0;
	unsigned long pos,textpos;
	struct tm *thattime;

	if(!_fpsysmgr)
	{
		EDEBUG(2,"_fpsysmgr NULL\n");
		return -1;
	}
	pos=sizeof(filemgr_header)+(_mgrhdr.nodenum-1)*sizeof(filemgr_node);	
	fseek(_fpsysmgr, pos, SEEK_SET);	
	
	m_node->datalen=datafh->length;
	
	//thattime=localtime(&datafh->create);
	//strftime(newnode->time, 32, "%m-%d %H:%M:%S", thattime);

	EDEBUG(FILESYSMGR_DEBUG_LVL,"new node: index:0x%x  datlen:0x%x\n", m_node->index,\
		 m_node->datalen);

	//strcpy(newnode->data_file,datafh->name);	
	fseek(_fpsysmgr, pos, SEEK_SET);	
	nbytes=fwrite(m_node, 1, sizeof(filemgr_node), _fpsysmgr);

	_mgrhdr.current_length+=datafh->length;
	
	fseek(_fpsysmgr, 0, SEEK_SET); 
	fwrite(&_mgrhdr, 1, sizeof(filemgr_header), _fpsysmgr);	
	fflush(_fpsysmgr);	
	
	EDEBUG(FILESYSMGR_DEBUG_LVL, "Now FileSys Len:0x%x nodenum:0x%x from 0x%x to 0x%x\n",\
		_mgrhdr.current_length,_mgrhdr.nodenum, _mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);	

	return nbytes;
}

int CFilesys_mgr::report(int fd)
{
	int index=0;
	int nodelen=sizeof(filemgr_node);
	filemgr_node tmpnode;
	int len=0;
	char *outbuff=new char[0x400];
	int nwrite=0;

	if(fd < 0)
	{
		printf("%s %d err: fd:%d \n", __func__, __LINE__, fd);
		return -1;		
	}

	sprintf(outbuff,"Filesys cap:0x%x datalen:0x%x recNum:0x%x nodelen:0x%x-0x%x\n",\
			_mgrhdr.device_capacity, _mgrhdr.current_length, _mgrhdr.nodenum,\
			_mgrhdr.nodeindex_start, _mgrhdr.nodeindex_stop);
	len=write(fd, outbuff, strlen(outbuff)+1);
	if(len !=(strlen(outbuff)+1))
	{
		printf("%s %d err: fd:0x%x len:0x%x\n", __func__, __LINE__, fd, len);
		return -1;
	}
	
	if(_mgrhdr.nodenum == 0)
	{
		return len;
	}

	fseek(_fpsysmgr, sizeof(filemgr_header), SEEK_SET);
	for(index=_mgrhdr.nodeindex_start;index<=_mgrhdr.nodeindex_stop;index++)
	{
		fread(&tmpnode, 1, nodelen, _fpsysmgr);
		sprintf(outbuff, "%s:\t file:%s \t data length:0x%x\n", 
			tmpnode.time, tmpnode.data_file, tmpnode.datalen);
		nwrite=write(fd, outbuff, strlen(outbuff)+1);
		if(nwrite !=(strlen(outbuff)+1))
		{
			printf("%s %d err: fd:%d len:0x%x\n", __func__, __LINE__, fd, nwrite);
			return len;
		}	
		len+=nwrite;
	}
	return len;
}

/*
 * =====================================================================================
 * end
 * =====================================================================================
 */

