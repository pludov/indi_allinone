/*
 * Lock.cpp
 *
 *  Created on: 25 nov. 2017
 *      Author: ludovic
 */

#include <Lock.h>

Lock::Lock(pthread_mutex_t * m) {
	this->mutex = m;
	this->acquireCount = 0;
}

Lock::~Lock() {
	// TODO Auto-generated destructor stub
	if (this->acquireCount) {
		pthread_mutex_unlock(mutex);
	}
}

void Lock::lock()
{
	acquireCount++;
	if (acquireCount == 1) {
		pthread_mutex_lock(mutex);
	}
}

void Lock::release()
{
	acquireCount --;
	if (acquireCount == 0) {
		pthread_mutex_unlock(mutex);
	}
}
