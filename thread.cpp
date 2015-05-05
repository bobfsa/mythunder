extern "C" {
#include <sys/time.h>
}
#include <stdio.h>

#include "thread.h"


Cthread::Cthread()
{
	started_ = false;
	detached_ = false;
	priority_ = PRIORITY_NORMAL;
	flag_=0;
}

Cthread::~Cthread()
{
}

int Cthread::start(int waiting)
{
	pthread_attr_t my_attr;
	int my_res = 0;

	//printf("waiting<%d>\n", waiting);
	if (pthread_attr_init(&my_attr) != 0)
	{
		printf("pthread_attr_init failed\n");
		return -1;
	}
	if (pthread_attr_setstacksize(&my_attr, 1<<20) != 0)
	{
		printf("pthread_attr_setstacksize failed\n");
		my_res = -1;
		goto err;

	}
	if (waiting == 1)
	{
		cond_.lock();
		flag_--;
		cond_.unlock();
	}

	if (priority_ != PRIORITY_NORMAL)
	{
//		int my_policy = SCHED_RR;
		int my_policy = SCHED_FIFO;

		printf("priority:%d\n", priority_);
		if ( pthread_attr_setschedpolicy(&my_attr, my_policy) != 0 )
		{
			printf("pthread_attr_setschedpolicy failed\n");
			my_res = -1;
			goto err;
		}
		if ( pthread_attr_getschedpolicy(&my_attr, &my_policy) != 0 )
		{
			printf("pthread_attr_getschedpolicy failed\n");
			my_res = -1;
			goto err;
		}
		printf("policy<%d>\n", my_policy);
		int my_max = sched_get_priority_max(my_policy);
		int my_min = sched_get_priority_min(my_policy);
		printf("min<%d>, max<%d>\n", my_min, my_max);
		if ( my_min == -1 || my_max == -1 )
		{
			printf("pthread_attr_getschedpolicy failed\n");
			my_res = -1;
			goto err;
		}
		struct sched_param my_param;
		if ( pthread_attr_getschedparam(&my_attr, &my_param) != 0 )
		{
			printf("pthread_attr_getschedpolicy failed\n");
			my_res = -1;
			goto err;
		}
		my_param.sched_priority = 101 /*my_min + (priority_ * (my_max - my_min)) / 100*/;
		/*if ( pthread_attr_setschedparam(&my_attr, &my_param) != 0 )
		{
			PDEBUG(0, "pthread_attr_getschedpolicy failed\n");
			my_res = -1;
			goto err;
		}*/
		if ( pthread_attr_setinheritsched(&my_attr, PTHREAD_EXPLICIT_SCHED) != 0)
		{
			printf("pthread_attr_setinheritsched failed\n");
			my_res = -1;
			goto err;
		}
	}

	started_ = true;
	if (pthread_create(&thread_, &my_attr, routines, this) != 0)
	{
		printf("pthread_create failed\n");
		started_ = false;
		my_res = -1;
	}
err:
	if ( pthread_attr_destroy(&my_attr) != 0 )
	{
		printf("pthread_attr_destroy failed\n");
		my_res = -1;
	}
	return my_res;
}

int Cthread::set_priority(thread_priority priority)
{
	//printf("enter\n");
	priority_ = priority;
	return 0;
}

int Cthread::cancel(void)
{
	//printf("enter\n");
	if (started_ == true)
		pthread_cancel(thread_);
	started_ = false;
	return 0;
}

int Cthread::exit(void)
{
	//printf("enter\n");
	if (started_ == true)
		pthread_exit(NULL);
	started_ = false;
	return 0;
}

int Cthread::detach(void)
{
	//printf("enter\n");
	if (started_ == true)
	{
		pthread_detach(thread_);
		detached_ = true;
	}
	return 0;
}

int Cthread::join(void)
{
	//printf("enter\n");
	if (started_)
	{
		void *pv;
		pthread_join(thread_, &pv);
	}
	return 0;
}

void *Cthread::routines(void *pv)
{
	//printf("enter\n");

	Cthread* my_pthread = (Cthread*)pv;
	if ( my_pthread != NULL )
		my_pthread->sub_routine();
	pthread_exit(NULL);
	//printf("leave\n");
}

int Cthread::stop()
{
	//printf("enter\n");
	if ( started_ == false)
		return -1;
	if ( detached_ == false )
		join();
	started_ = false;
	return 0;
}
	
int Cthread::suspend(void)
{
	//printf("enter\n");
	if (started_ == false)
		return -1;
	cond_.lock();
	flag_--;
	cond_.unlock();
	return 0;
}
	
int Cthread::resume(void)
{
	//printf("enter\n");
	if (started_ == false)
	{
		start();
	}
	else
	{
		cond_.lock();
		flag_++;
		cond_.signal();
		cond_.unlock();
	}
	return 0;
}

int Cthread::wait(void)
{
	//printf( "enter\n");
	if (started_ == false)
		return -1;
	cond_.lock();
	while (flag_ < 0)
	{
		cond_.wait();
	}
	cond_.unlock();
	return 0;
}
/*
 * =====================================================================================
 * end
 * =====================================================================================
 */
