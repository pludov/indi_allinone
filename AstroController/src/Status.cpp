/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "Status.h"

#define STATUS_INTERVAL 10


Status::Status():
	Scheduled::Scheduled(F("Status")),
	statusGroup(Symbol(F("Status"), 0)),
	uptime(statusGroup, F("UPTIME"), F("uptime"), VECTOR_READABLE | VECTOR_WRITABLE),
	uptimeValue(&uptime, F("UPTIME_VALUE"), F("uptime (s)"), 0, 1000000, 1)
{
	this->nextTick = UTime::now();
	this->priority = 2;
	this->tickExpectedDuration = US(100);
}

Status::~Status()
{}

void Status::tick()
{
	uptimeValue.setValue(uptimeValue.getValue() + STATUS_INTERVAL);
	this->nextTick += LongDuration::seconds(STATUS_INTERVAL);
}

