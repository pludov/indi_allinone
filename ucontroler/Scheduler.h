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
private:
	Scheduled ** scheduleds;

public:
	// expect a null terminated list of scheduled
	Scheduler(Scheduled ** scheduleds);
	void loop();
};

#endif /* SCHEDULER_H_ */
