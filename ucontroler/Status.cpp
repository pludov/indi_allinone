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
#include "debug.h"
#include "SerialIO.h"

#include "IndiNumberVector.h"
#include "IndiVectorGroup.h"
#include "IndiFloatVectorMember.h"

extern PWMResistor resistor;

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79

IndiVectorGroup statusGroup(F("Status"));
IndiNumberVector uptime(&statusGroup, F("UPTIME"), F("Time since power up/reset"));
IndiFloatVectorMember uptimeValue(&uptime, F("UPTIME_VALUE"), F("Time since power up/reset (s)"), 0, 1e36);

//
//static int pendingWrite()
//{
//	Serial.availableForWrite()
//	return (unsigned int)(SERIAL_TX_BUFFER_SIZE + Serial._tx_buffer_head - Serial._tx_buffer_tail) % SERIAL_TX_BUFFER_SIZE;
//}

Status::Status()
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
	uptimeValue.setValue(millis() / 1000.0);
	this->nextTick += LongDuration::seconds(10);
}


void writeHex(char * buff, int length, uint32_t value)
{
	while(length > 0) {
		length--;
		int i = value & 15;
		if (i >= 10) {
			buff[length] = 'A' + i - 10;
		} else {
			buff[length] = '0' + i;
		}
		value = value >> 4;
	}
}

uint32_t readHex(String & val)
{
	uint32_t result = 0;
	int shift = 0;
	for(int i = val.length() - 1; i>= 0; --i)
	{
		char c = val[i];
		uint32_t v;
		if (c >= 'a' && c <= 'f') {
			v = 10 + c - 'a';
		} else if (c >= 'A' && c <= 'F') {
			v = 10 + c - 'A';
		} else if (c >= '0' && c <= '9') {
			v = c - '0';
		} else {
			continue;
		}
		result = result | (v << shift);
		shift += 4;
	}
	return result;
}

static void writeBlank(char * buffer, int length)
{
	for(int i = 0;i < length; ++i)
	{
		buffer[i] = ' ';
	}
}

static void writeTemp(char * buffer, float temp)
{
	if (temp == NAN) {
		writeBlank(buffer, 4);
		return;
	}
	unsigned long scaledValue = round(temp * 100 + 32768);
	writeHex(buffer, 4, scaledValue);
}

static void writeHum(char * buffer, float hum)
{
	if (hum == NAN) {
		writeBlank(buffer, 3);
	}
	unsigned long scaledValue = round(hum * 40.95);
	writeHex(buffer, 3, scaledValue);
}

static void writeVolt(char * buffer, float hum)
{
	if (hum == NAN) {
		writeBlank(buffer, 3);
	}
	unsigned long scaledValue = round(hum * 100.0);
	writeHex(buffer, 3, scaledValue);
}

Payload Status::getStatusPayload()
{
	Payload s;
/*	writeHex(s.motor, 5, motor.getCurrentPosition());
	writeHex(s.motorState, 1, motor.isMoving() ? 1 : 0);

	writeTemp(s.scopeTemp, scopeTemp.lastValue);
	writeTemp(s.extTemp, meteoTemp.lastTemperature());
	writeHum(s.extHum, meteoTemp.lastHumidity());

	writeVolt(s.battery, voltmeter.lastValue());
	writeHex(s.heater, 2, resistor.pct);
	writeHex(s.filterwheel, 5, filterWheelMotor.getCurrentPosition());
	s.filterwheelState = filterWheelMotor.getProtocolStatus();*/
	return s;
}

void Status::sendStatus()
{
#ifdef DEBUG
	Serial.println(F("MMMMMS°sco°ext%%%VVVHHFFFFFS"));
#endif


#ifndef NO_STATUS
	Payload s = getStatusPayload();
	serialIO.sendPacket('S', (uint8_t*)&s, sizeof(s));
#endif

#ifdef DEBUG
	Serial.println("");
#endif
}



