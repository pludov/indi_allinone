/*
 * Scheduled.cpp
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */

#include <Arduino.h>
#include "Scheduled.h"
#include "Scheduler.h"

Scheduled::Scheduled()
{
	this->nextTick = UTime::never();
	this->priority = 0;
	this->tickExpectedDuration = US(0);
	this->nextScheduled = Scheduler::instance().first;
	Scheduler::instance().first = this;
}

Scheduled::~Scheduled()
{
	// FIXME: remove from scheduler not implemented
}

