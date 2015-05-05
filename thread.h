#ifndef __THREAD_H__
#define __THREAD_H__

/*
 * =====================================================================================
 * system header
 * =====================================================================================
 */
#include <algorithm>
#include <list>
#include <vector>
#include <pthread.h>

#include "criticalsection.h"


/*
 * =====================================================================================
 * local header
 * =====================================================================================
 */


/*
 * =====================================================================================
 * type Definition
 * =====================================================================================
 */
enum thread_priority
{
	PRIORITY_NORMAL = 5,
	PRIORITY_IDLE = 15,
	PRIORITY_HIGH = 30,
};

/** The Cthread class sets up, maintains and manages linux pthread 
 * 
 */
class Cthread
{
protected:
	/**
	 * @brief       set thread schedule priority
	 * @retval      0 for success.
	 */
	int set_priority(thread_priority priority);
	inline int get_priority(void)
	{
		return priority_;
	}
	/**
	 * @brief       suspend current thread
	 *
	 */
	int suspend(void);
	/**
	 * @brief       resume from suspend status
	 * @remarks     suspend should be called before
	 *
	 * @retval      0 for success.
	 */
	int resume(void);
	/**
	 * @brief       make thread join and wait for free resource
	 *
	 *
	 * @remarks     detach should not be called when start
	 *
	 * @retval      0 for success.
	 */
	int join(void);
	/**
	 * @brief       cancel thread execution
	 *
	 *
	 * @remarks     start should be called before
	 *
	 * @retval      0 for success.
	 */
	int cancel(void);
	/**
	 * @brief       exit thread and terminate
	 *
	 *
	 * @remarks     start should be called before
	 *
	 * @retval      0 for success.
	 */
	int exit(void);
	/**
	 * @brief       detach thread to make it independable
	 *
	 *
	 * @remarks     start should be called before
	 *
	 * @retval      0 for success.
	 */
	int detach(void);
	/**
	 * @brief       wait until next wakeup
	 *
	 *
	 * @remarks     start should be called before
	 *
	 * @retval      0 for success.
	 */
	int wait(void);

	/**
	 * @brief       start thread
	 *
	 *
	 * @param[in]   waiting		start thread and suspend it
	 *
	 * @retval      0 for success.
	 */
	virtual int start(int waiting = 0);
	/**
	 * @brief       stop thread
	 *
	 *
	 * @remarks     start should be called before
	 *
	 * @retval      0 for success.
	 */
	virtual int stop(void);
	/**
	 * @brief       get thread unique id
	 *
	 *
	 * @retval      0 for success.
	 */
	pthread_t get_threadid(void)
	{
		return thread_;
	}
protected:
	/**
	 * @brief       thread main entrance
	 *
	 *
	 * @remarks     can be overlapped
	 */
	virtual void *sub_routine(void) = 0;
	/**
	 * @brief       static main entrance
	 *
	 */
	static void *routines(void *pv);

protected:
	Cthread();
	~Cthread();

private:
	/*! thread priority enumeration */
	thread_priority priority_;
	/*! thread id variable */
	pthread_t thread_;
	/*! thread start or not flag */
	bool started_;
	/*! thread is joined or not */
	bool detached_;
	/*! thread suspend flag */
	int flag_;
	/*! thread suspend condition */
	ccond cond_;
};

/*
 * =====================================================================================
 * end
 * =====================================================================================
 */
#endif // __THREAD_H__
