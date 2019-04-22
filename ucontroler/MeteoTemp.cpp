/*
 * meteotemp.cpp
 *
 *  Created on: 4 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "CommonUtils.h"
#include "MeteoTemp.h"
#include "Status.h"


// Interval between read attempts (usec)
#define READ_INTERVAL 5000000


// how many timing transitions we need to keep track of. 2 * number bits + extra
#define MAXTIMINGS 85



MeteoTemp::MeteoTemp(uint8_t pin, uint8_t type) : Scheduled(F("MeteoTemp")),
	group(Symbol(F("METEO"))),
	statusVec(group, Symbol(F("Meteo")), F("Measure")),
	temp(&statusVec, F("METEO_TEMP"), F("Temp"), -100,100,1),
	hum(&statusVec, F("METEO_HUM"), F("Hum (%)"), 0, 100, 1),
	dewPoint(&statusVec, F("METEO_DEW_POINT"), F("Dew Point"), -100, 100, 1)
{
	_pin = pin;
	_type = type;
	_count = 27;

	dewPointValue = NAN;
	hasData = false;

	// set up the pins!
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, HIGH);

	setInvalid();
	prepareStep1(true);
}

void MeteoTemp::setInvalid()
{
	temp.setValue(-1000);
	hum.setValue(-1);
	dewPoint.setValue(-1000);
}

void MeteoTemp::setValid()
{
	temp.setValue(tempValue);
	hum.setValue(humValue);
	dewPoint.setValue(dewPointValue);
}

void MeteoTemp::tick()
{
	DEBUG(F("MeteoTemp::tick"));
	switch(nextStep) {
	case 1:
		doStep1();
		return;
	case 2:
		doStep2();
		return;
	case 3:
		doStep3();
		return;
	}
}

MeteoTemp::~MeteoTemp() {
	// TODO Auto-generated destructor stub
}


float MeteoTemp::extractTemperature() {
	float f;

	if (hasData) {
		switch (_type) {
		case DHT11:
			f = data[2];

			return f;
		case DHT22:
		case DHT21:
			f = data[2] & 0x7F;
			f *= 256;
			f += data[3];
			f /= 10;
			if (data[2] & 0x80)
				f *= -1;


			return f;
		}
	}
	return NAN;
}


float MeteoTemp::extractHumidity(void) {
	float f;
	if (hasData) {
		switch (_type) {
		case DHT11:
			f = data[0];
			return f;
		case DHT22:
		case DHT21:
			f = data[0];
			f *= 256;
			f += data[1];
			f /= 10;
			return f;
		}
	}
	return NAN;
}

void MeteoTemp::prepareStep1(bool first)
{
	nextTick = UTime::now() + (first ? 0 : READ_INTERVAL);
	priority = 3;
	tickExpectedDuration = US(50);
	nextStep = 1;
}


void MeteoTemp::doStep1()
{
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, HIGH);
	prepareStep2();
}

void MeteoTemp::prepareStep2()
{
	nextTick = UTime::now() + 250000;
	priority = 0;
	tickExpectedDuration = US(100);
	nextStep = 2;
}


void MeteoTemp::doStep2()
{
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin, LOW);
	prepareStep3();
}

void MeteoTemp::prepareStep3()
{
	nextTick = UTime::now() + 20000;
	priority = 0;
	// Jusqu'à 5 ms (pas de chance !)
	tickExpectedDuration = MS(5);
	nextStep = 3;
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
// Accurate to +-1 degree C as long as the relative humidity is above 50%
static double dewPointFast(double celsius, double humidity) {
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity / 100);
  double Td = (b * temp) / (a - temp);
  return Td;
}

// calculates dew point
// input:   humidity [%RH], temperature in C
// output:  dew point in C
// use this where RH is < 50%
static float calc_dewpoint(float t, float h) {
  float logEx, dew_point;
  logEx = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);
  dew_point = (logEx - 0.66077) * 237.3 / (0.66077 + 7.5 - logEx);
  return dew_point;
}

void MeteoTemp::doStep3()
{
	int i, j = 0, counter = 0;
	uint8_t laststate = HIGH;
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;

#ifdef DEBUG
	unsigned long begin = micros();
#endif
	pinMode(_pin, OUTPUT);

	// FIXME: noInterrupts for > 2ms = very bad practice
	noInterrupts();
	digitalWrite(_pin, HIGH);
	delayMicroseconds(40);
	pinMode(_pin, INPUT);

	// read in timings
	for ( i=0; i< MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(_pin) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(_pin);

		if (counter == 255) break;

		// ignore first 3 transitions
		if ((i >= 4) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > _count)
				data[j/8] |= 1;
			j++;
		}

	}

	interrupts();

	bool isOk = (j >= 40) &&
			(data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF));
	hasData = isOk;

//	status.needUpdate();

	unsigned long duration = micros() - begin;
	DEBUG(F("DHT="), isOk, F(" took:"), duration);
	DEBUG(F("j="), j);


	if (isOk) {
		tempValue = extractTemperature();
		humValue = extractHumidity();
		dewPointValue = calc_dewpoint(tempValue, humValue);
		DEBUG(F("Temp="),tempValue, "C");
		DEBUG(F("Hum="),humValue, "%");
		DEBUG(F("DewPoint="),dewPointValue, "C");

		setValid();
	} else {
		tempValue = NAN;
		humValue = NAN;
		dewPointValue = NAN;
		setInvalid();
	}

	prepareStep1(false);
}

