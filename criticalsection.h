#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

/*
 * =====================================================================================
 * system header
 * =====================================================================================
 */
#include <pthread.h>
#include <time.h>
#include "osal.h"
/*
 * =====================================================================================
 * class prototype
 * =====================================================================================
 */
/** The Ccritical_section class define mutex encapsulation
 * 
 */
class cmutex {
public:
	cmutex() {
		pthread_mutexattr_t mutex_attribute;
		pthread_mutexattr_init(&mutex_attribute);
		pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&mutex_, &mutex_attribute);
	}
	~cmutex() {
		pthread_mutex_destroy(&mutex_);
	}
	inline void lock() {
		pthread_mutex_lock(&mutex_);
	}
	inline void unlock() {
		pthread_mutex_unlock(&mutex_);
	}
public:
	pthread_mutex_t mutex_;
};

/** The Ccritical_scope class for serializing exection through a scope
 *
 */
class mutex_scope {
public:
	mutex_scope(cmutex *pcrit) {
		pcrit_ = pcrit;
		pcrit_->lock();
	}
	~mutex_scope() {
		pcrit_->unlock();
	}
private:
	cmutex *pcrit_;
};

/** The Ccritical_condition class define condition encapsulation
 *
 */
class ccond
	: public cmutex
{
public:
	ccond():cmutex()
	{
		pthread_cond_init(&cond_, NULL);
	}
	~ccond()
	{
		pthread_cond_destroy(&cond_);
	}
	inline void wait()
	{
		pthread_cond_wait(&cond_, &mutex_);
	}
	inline void signal()
	{
		pthread_cond_signal(&cond_);
	}
public:
	pthread_cond_t cond_;
};


class ccount_sem
{
public:
	ccount_sem()
	{
		sem_init(&sem_, 0, 0);
	}
	~ccount_sem()
	{
		sem_destroy(&sem_);
	}
	void  ccount_give()
	{
		sem_post(&sem_);	
	}
	void ccount_take()
	{
		sem_wait(&sem_);	
	}

private:
	sem_t sem_; 	
};

/*
 * =====================================================================================
 * end
 * =====================================================================================
 */
#endif // __CRITICALSECTION_H__
