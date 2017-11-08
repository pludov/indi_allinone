/*
 * FilterWheelMotor.cpp
 *
 *  Created on: 29 nov. 2015
 *      Author: utilisateur
 */

#include <Arduino.h>
#include "FilterWheelMotor.h"
#include "Status.h"
#include "debug.h"

#define POS_INVALID ((uint32_t)-1L)

#define PIN_HALL_POWER A4
#define PIN_HALL_READ A5

FilterWheelMotor::FilterWheelMotor(const uint8_t * pins, uint8_t positionConfigId, int fastestPerHalfStep) :
	Motor::Motor(pins, positionConfigId, fastestPerHalfStep)
{
	calibrating = false;
	calibrationTarget = -1;
	calibrationState = false;
	pinMode(PIN_HALL_POWER, OUTPUT);
	pinMode(PIN_HALL_READ, INPUT);
}

FilterWheelMotor::~FilterWheelMotor() {
}

void FilterWheelMotor::loadConfigPosition()
{
	Motor::loadConfigPosition();
	if (currentPosition == POS_INVALID) {
		loadPosition(0);
		calibrationState = true;
	}
}


void FilterWheelMotor::startCalibration(unsigned long target)
{
	calibrating = true;
	calibrationTarget = target;
	// Start the hall sensor
	setHallStatus(true);
	loadPosition(50000);
	setTargetPosition(0);

	status.needUpdate();
}

bool FilterWheelMotor::readHall()
{
//	return currentPosition == 46700;
	return digitalRead(PIN_HALL_READ) == LOW;
}

void FilterWheelMotor::setHallStatus(bool onOff)
{
	digitalWrite(PIN_HALL_POWER, onOff ? HIGH : LOW);
}


void FilterWheelMotor::targetPositionReached()
{
	if (calibrating) {
		calibrating = false;
#ifdef DEBUG
		Serial.println(F("calibration failed"));
#endif

		this->speedLevel = 0;
		this->clearOutput();
		this->nextTick = UTime::never();
		status.needUpdate();

		positionConfig().position = POS_INVALID;
		config.commitStorage(positionConfigId);
		setHallStatus(false);
		status.needUpdate();
		return;
	}
	Motor::targetPositionReached();
}

void FilterWheelMotor::tick()
{
	if (calibrating) {
		if (readHall()) {
			setHallStatus(false);
			calibrating = false;
			calibrationState = false;
			loadPosition(0);
			status.needUpdate();
			setTargetPosition(calibrationTarget);
#ifdef DEBUG
			Serial.println(F("calibration succeded"));
#endif
			return;

		}
	}
	Motor::tick();
}

char FilterWheelMotor::getProtocolStatus()
{
	if (isMoving()) {
		if (calibrating) {
			return FILTER_WHEEL_STATUS_CALIBRATION_RUNNING;
		} else {
			return FILTER_WHEEL_STATUS_MOVING;
		}
	} else if (lastCalibrationFailed()) {
		return FILTER_WHEEL_STATUS_CALIBRATION_FAILED;
	} else {
		return FILTER_WHEEL_STATUS_IDLE;
	}
}

