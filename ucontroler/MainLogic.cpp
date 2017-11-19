/*
 * MainLogic.cpp
 *
 *  Created on: 7 fï¿½vr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "MainLogic.h"
#include "MeteoTemp.h"
#include "scopetemp.h"
#include "pwmresistor.h"
#include "Status.h"
#include "Config.h"
#include "debug.h"

extern PWMResistor resistor;

MainLogic::MainLogic():Scheduled::Scheduled(F("MainLogic")) {
	this->nextTick = UTime::now();
	this->priority = 1;
	this->tickExpectedDuration = MS(1);
}

MainLogic::~MainLogic() {
}

void MainLogic::temperatureUpdated()
{
	this->nextTick = UTime::now();
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
// Accurate to +-1 degree C as long as the relative humidity is above 50%
double dewPointFast(double celsius, double humidity) {
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
float calc_dewpoint(float t, float h) {
  float logEx, dew_point;
  logEx = 0.66077 + 7.5 * t / (237.3 + t) + (log10(h) - 2);
  dew_point = (logEx - 0.66077) * 237.3 / (0.66077 + 7.5 - logEx);
  return dew_point;
}

// Appellï¿½ toutes les 20 secondes.
// On crï¿½e des pas de 8 sur le pwm, il y a donc 32 pas, soit 32*20 = 10 minutes pour eteindre
void MainLogic::updatePwm()
{
	float scopeValue = scopeTemp.lastValue;
	float extTemp = meteoTemp.lastTemperature();
	float extHum = meteoTemp.lastHumidity();
	if (isnan(scopeValue)) {
#ifdef DEBUG
		Serial.println("No scope value");
#endif
		return;
	} else if (isnan(extTemp) || isnan(extHum)) {
#ifdef DEBUG
		Serial.println("no ext value");
#endif
		return;
	}

	float dewPoint;
	if (extHum < 50) {
		dewPoint = calc_dewpoint(extTemp, extHum);
	} else {
		dewPoint = dewPointFast(extTemp, extHum);
	}
#ifdef DEBUG
	Serial.print("dewpoint:");
	Serial.println(dewPoint);
#endif
	int pwm = resistor.pct;
	float target = config.getTargetDeltaTemp();
	if (scopeValue < dewPoint + target - 0.5) {
		pwm = 255;
	} else if (scopeValue < dewPoint + target - 0.1) {
		pwm += config.getPwmStep();
	} else if (scopeValue > dewPoint + target + 0.1) {
		pwm-= config.getPwmStep();
	}
	if (pwm < 0) {
		pwm = 0;
	}
	if (pwm > 255) {
		pwm = 255;
	}
	if (resistor.pct != pwm) {
		resistor.setLevel(pwm);
		status.needUpdate();
	}
}

void MainLogic::tick()
{
	updatePwm();
	this->nextTick += LongDuration::seconds(20000);
}

