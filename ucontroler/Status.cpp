/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "Status.h"
#include "MainLogic.h"
#include "MeteoTemp.h"
#include "Motor.h"
#include "FilterWheelMotor.h"
#include "voltmeter.h"
#include "scopetemp.h"
#include "pwmresistor.h"

#include "IndiNumberVector.h"
#include "IndiFloatVectorMember.h"
#include "IndiTextVector.h"
#include "IndiTextVectorMember.h"

extern PWMResistor resistor;

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

Symbol statusGroup(F("Status"));
IndiNumberVector uptime(statusGroup, F("UPTIME"), F("Time since power up/reset"), VECTOR_READABLE | VECTOR_WRITABLE);
IndiFloatVectorMember uptimeValue(&uptime, F("UPTIME_VALUE"), F("Time since power up/reset (s)"), 0, 1e36, 1);

IndiTextVector freeText(statusGroup, F("FREETEXT"), F("Free text"), VECTOR_READABLE|VECTOR_WRITABLE);
IndiTextVectorMember freeTextValue(&freeText, F("FREETEXT_VALUE"), F("Free text"), 64);

//
//static int pendingWrite()
//{
//	Serial.availableForWrite()
//	return (unsigned int)(SERIAL_TX_BUFFER_SIZE + Serial._tx_buffer_head - Serial._tx_buffer_tail) % SERIAL_TX_BUFFER_SIZE;
//}

Status::Status():Scheduled::Scheduled(F("Status"))
{
	this->nextTick = UTime::now();
	this->priority = 2;
	this->tickExpectedDuration = MS(1);
}

Status::~Status()
{}


void Status::needUpdate()
{
	UTime n = UTime::now();
	if (this->nextTick > n) {
		this->nextTick = n;
	}
}

void Status::tick()
{
	uptimeValue.setValue(uptimeValue.getDoubleValue() + 10.0);
	this->nextTick += LongDuration::seconds(10);
}

