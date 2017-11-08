/*
 * voltmeter.cpp
 *
 *  Created on: 7 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "debug.h"
#include "voltmeter.h"
#include "MainLogic.h"
#include "Status.h"
#include "Config.h"

Voltmeter::Voltmeter(uint8_t pin) {
	this->priority = 2;
	this->tickExpectedDuration = US(250);
	this->pin = pin;
	this->nextTick = UTime::now();
	this->lastv = -1;
	analogReference(INTERNAL);
}

Voltmeter::~Voltmeter() {
}

void Voltmeter::tick()
{
	unsigned int rawV = analogRead(pin);

	// 627 => 11.89
	unsigned int v = ((unsigned long)rawV * 100 * config.storedVoltmeter().voltmeter_mult) >> 17;
#ifdef DEBUG
	Serial.print("V:");
	Serial.println(v);
#endif
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
