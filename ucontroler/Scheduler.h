/*
 * scheduler.h
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */



#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "Scheduled.h"

class Scheduler
{
	friend class Scheduled;
private:
	
	Scheduled * first;

public:
	// expect a null terminated list of scheduled
	Scheduler();
	void loop();

	static Scheduler & instance() __attribute__ ((const));
};

#endif /* SCHEDULER_H_ */
