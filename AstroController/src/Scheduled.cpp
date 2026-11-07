/*
 * Scheduled.cpp
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */

#include <Arduino.h>
#include "CommonUtils.h"
#include "Scheduled.h"
#include "Scheduler.h"

Scheduled::Scheduled(const Symbol & debug)
	: debugName(debug)
{
	this->debugName = debug;
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

void Scheduled::debugPerf() const
{
	DEBUG(F("Schedule for "), debugName);
	auto levelCount = perf.getLevelCount();
	for(int i = 0; i < levelCount - 1; i++) {
		DEBUG(F("  "), perf.getUsecLevel(i), F(" usec  "), perf.getSampleCount(i));
	}
	DEBUG(F("  > "), perf.getUsecLevel(levelCount - 1), F(" usec  "), perf.getSampleCount(levelCount - 1));
}