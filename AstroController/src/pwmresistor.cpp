/*
 * pwmresistor.cpp
 *
 *  Created on: 5 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "pwmresistor.h"

#include "TimerOne.h"

// On travaille en ~50Hz. Il faut un multiple de 255 de préférence
# define BASE_TIME 20400


PWMResistor::PWMResistor(uint8_t pin) {
	this->pin = pin;
	this->pct = 0;

	// Periode de 0.1s.
	Timer1.initialize(100000);
	Timer1.start();
	Timer1.pwm(this->pin, 0);
}

void PWMResistor::setLevel(uint8_t level)
{
	if (this->pct == level) {
		return;
	}
	this->pct = level;
	Timer1.initialize(100000);
	Timer1.pwm(this->pin, level < 255 ? ((uint16_t)level) << 2 : 1023);
}

PWMResistor::~PWMResistor() {
}

