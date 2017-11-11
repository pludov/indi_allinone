/*
 * scheduled.h
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */

#ifndef SCHEDULED_H_
#define SCHEDULED_H_

#define  NEVER  -0x80000000

#include "utime.h"

class Scheduled
{
	friend class Scheduler;
	Scheduled * nextScheduled;
	
		
protected:
	// When is the next tick scheduled
	UTime nextTick;
	// How long will it last (in 16ms) maxi = 1s
	ShortDuration tickExpectedDuration;
	// priority 0 = don't be late. Other = can be late
	int priority;

	Scheduled();
	
	virtual void tick() = 0;
	virtual ~Scheduled() = 0;
};



#endif /* SCHEDULED_H_ */
