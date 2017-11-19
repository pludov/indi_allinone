/*
 * scopetemp.cpp
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */

#include "debug.h"
#include "scopetemp.h"
#include "MainLogic.h"
#include "Status.h"

// 5 seconde for reads
#define REFRESH_DEFAULT_INTERVAL  5000000

#define WAIT_BEFORE_READ 100000
// Retry every 50ms
#define WAIT_BETWEEN_READ_RETRY 50000
// Fail after this number of retry (if the device is still not ready)
#define MAX_READ_RETRY 40

// Duration in ~ms of a request
#define REQUEST_DURATION MS(6)
// Duration in ~ms of a read
#define READ_TRY_DURATION MS(18)

ScopeTemp::ScopeTemp(OneWire * oneWire): Scheduled::Scheduled(F("ScopeTemp")), dallas(oneWire)
{
	dallas.setWaitForConversion(false);
	this->priority = 2;
	if (searchSensor()) {
		this->nextTick = UTime::now();
		this->tickExpectedDuration = REQUEST_DURATION;
		requested = false;
	} else {
#ifdef DEBUG
		Serial.println(F("dallas sensor not found"));
#endif
		this->nextTick = UTime::never();
	}
	this->lastValue = NAN;
}

ScopeTemp::~ScopeTemp() {
}

bool ScopeTemp::isAvailable()
{
	return !this->nextTick.isNever();
}

bool ScopeTemp::searchSensor()
{
	dallas.begin();
	if (dallas.getDeviceCount() < 1) {
		return false;
	}
	if (!dallas.getAddress(sensorAddress, 0))
	{
		return false;
	}
	dallas.setResolution(sensorAddress, 10);
	return true;
}

void ScopeTemp::tick()
{
#ifdef DEBUG
	long now = micros();
#endif
	if (!requested) {
		dallas.requestTemperatures();
		requested++;
		this->nextTick = UTime::now() + WAIT_BEFORE_READ;
		this->tickExpectedDuration = READ_TRY_DURATION;
	} else {
//		if (!dallas.isConversionAvailable(sensorAddress)) {
//			requested++;
//			if (requested >= MAX_READ_RETRY) {
//#ifdef DEBUG
//				Serial.println("Read temp failed");
//#endif
//				requested = 0;
//			} else {
//				this->nextTick = UTime::now() + WAIT_BETWEEN_READ_RETRY;
//			}
//			return;
//		}

		lastValue = dallas.getTempC(sensorAddress);
		if (lastValue == DEVICE_DISCONNECTED_C) {
			lastValue = NAN;
		}
#ifdef DEBUG
		Serial.print(F("Scope temp: "));
		Serial.print(lastValue);
		Serial.print(F(" after "));
		Serial.print(requested - 1);
		Serial.println(F("retries"));
#endif
		this->nextTick = UTime::now() + REFRESH_DEFAULT_INTERVAL;
		this->tickExpectedDuration = REQUEST_DURATION;
		requested = 0;
		mainLogic.temperatureUpdated();
		status.needUpdate();
	}

#ifdef DEBUG
	now = micros() - now;
	Serial.print(F("dallas:"));
	Serial.println(now);
#endif
}
