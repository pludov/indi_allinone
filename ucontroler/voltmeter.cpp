/*
 * voltmeter.cpp
 *
 *  Created on: 7 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "voltmeter.h"
#include "MainLogic.h"
#include "Status.h"
#include "Config.h"

Voltmeter::Voltmeter(uint8_t pin) : Scheduled::Scheduled(F("Voltmeter")) {
	this->priority = 2;
	this->tickExpectedDuration = US(250);
	this->pin = pin;
	this->nextTick = UTime::now();
	this->lastv = -1;
#ifdef __AVR_ATmega2560__
	analogReference(INTERNAL1V1);
#else
	analogReference(INTERNAL);
#endif
}

Voltmeter::~Voltmeter() {
}

void Voltmeter::tick()
{
	unsigned int rawV = analogRead(pin);

	// 627 => 11.89
	unsigned int v = ((unsigned long)rawV * 100 * config.storedVoltmeter().voltmeter_mult) >> 17;

	DEBUG(F("V:"), v);

	this->nextTick += LongDuration::seconds(2);

	if (this->lastv != v) {
		this->lastv = v;
		status.needUpdate();
	}
}

float Voltmeter::lastValue()
{
	if (this->lastv == -1) return NAN;
	return this->lastv / 100.0;

}
