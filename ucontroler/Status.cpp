/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
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


// IndiTextVector freeText(statusGroup, F("FREETEXT"), F("Free text"), VECTOR_READABLE|VECTOR_WRITABLE);
// IndiTextVectorMember freeTextValue(&freeText, F("FREETEXT_VALUE"), F("Free text"), 64);

//
//static int pendingWrite()
//{
//	Serial.availableForWrite()
//	return (unsigned int)(SERIAL_TX_BUFFER_SIZE + Serial._tx_buffer_head - Serial._tx_buffer_tail) % SERIAL_TX_BUFFER_SIZE;
//}


#ifdef MORE_DEBUG
uint8_t flag = 0;
#define STATUS_INTERVAL 2
#else
#define STATUS_INTERVAL 10
#endif


Status::Status():
	Scheduled::Scheduled(F("Status")),
	statusGroup(Symbol(F("Status"), 0)),
	uptime(statusGroup, F("UPTIME"), F("uptime"), VECTOR_READABLE | VECTOR_WRITABLE),
	uptimeValue(&uptime, F("UPTIME_VALUE"), F("uptime (s)"), 0, 1000000, 1)
{
	this->nextTick = UTime::now();
	this->priority = 2;
	this->tickExpectedDuration = MS(1);
#ifdef MORE_DEBUG
	pinMode(13, OUTPUT);
#endif
}

Status::~Status()
{}

void Status::tick()
{
	uptimeValue.setValue(uptimeValue.getValue() + STATUS_INTERVAL);
	this->nextTick += LongDuration::seconds(STATUS_INTERVAL);
#ifdef MORE_DEBUG
	flag = !flag;
	digitalWrite(13, flag ? HIGH : LOW);
#endif
}

