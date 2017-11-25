/*
 * Lock.h
 *
 *  Created on: 25 nov. 2017
 *      Author: ludovic
 */

#ifndef SOURCE_DIRECTORY__LOCK_H_
#define SOURCE_DIRECTORY__LOCK_H_

#include <pthread.h>

/*
 * Handle auto release of pthread_mutex in case of exceptions
 * Must not be shared accors threads.
 */
class Lock {
	pthread_mutex_t * mutex;
	int acquireCount;
public:
	Lock(pthread_mutex_t * m);
	~Lock();

	void lock();
	void release();
};

#endif /* SOURCE_DIRECTORY__LOCK_H_ */
