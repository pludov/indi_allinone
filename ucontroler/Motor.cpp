#include <Arduino.h>

#include "CommonUtils.h"

#include "Motor.h"
#include "Status.h"

#define intervalBetweenProgress ((unsigned long int)500000)

// lookup table to drive motor control board
const uint8_t stepPattern[8] = { B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001 };


static inline int8_t sgn(long val) {
	if (val < 0) return -1;
	if (val==0) return 0;
	return 1;
}

Motor::Motor(const uint8_t * pins, const Symbol & debug, int fastestPerHalfStepDesc, int fastestPerHalfStepAsc)
	:Scheduled(debug)
{
	this->fastestPerHalfStepDesc = fastestPerHalfStepDesc;
	this->fastestPerHalfStepAsc = fastestPerHalfStepAsc;
	// What is the duration ?
	this->tickExpectedDuration = US(150);
	// Don't be late please
	this->priority = 0;
	this->positionBase = 0;
	this->currentPosition = 1000;
	this->targetPosition = 1000;
	this->intermediateTargetPosition = this->targetPosition;
	this->motorPins = pins;
	this->nextTick = UTime::never();
	this->speedLevel = 0;
	for (int i = 0; i < 4; i++) {
		pinMode(motorPins[i], OUTPUT);
	}
	clearOutput();
}

//------------------------------------------------------------------
// Set output pins for stepper
//------------------------------------------------------------------
void Motor::setOutput(int out) {
	out += positionBase;
	for (int i = 0; i < 4; i++) {
		digitalWrite(motorPins[i], bitRead(stepPattern[out], i));
	}
}
//------------------------------------------------------------------

//------------------------------------------------------------------
// Clear output pins for stepper
// To ensure they are not left in an on state after movement
//------------------------------------------------------------------
void Motor::clearOutput() {
	for (int i = 0; i < 4; i++) {
		digitalWrite(motorPins[i], 0);
	}
}

void Motor::loadPosition(unsigned long newPosition)
{
	this->loadPosition(newPosition, positionBase);
}

void Motor::loadPosition(unsigned long newPosition, uint8_t newPositionBase)
{
	this->targetPosition = newPosition;
	this->intermediateTargetPosition = newPosition;
	this->currentPosition = newPosition;
	this->positionBase = newPositionBase;
}

unsigned long Motor::getCurrentPosition()
{
	return this->currentPosition;
}

unsigned long Motor::getTargetPosition()
{
	return this->targetPosition;
}

void Motor::setTargetPosition(unsigned long newPosition) {
	this->setTargetPosition(newPosition, newPosition);
}

void Motor::setTargetPosition(unsigned long newPosition, unsigned long newIntermediateTargetPosition)
{
	if (newIntermediateTargetPosition == this->currentPosition) {
		newIntermediateTargetPosition = newPosition;
	}

	if (this->targetPosition == this->currentPosition && this->targetPosition == this->intermediateTargetPosition) {
		// We are idle.
		this->speedLevel = 0;
		if (newPosition == this->targetPosition && newIntermediateTargetPosition == this->targetPosition) {
			// ok, nothing to do then
			return;
		}
		// We are idle. start the move within one ms...
		UTime startAt = UTime::now() + 1000;
		if (this->nextTick.isNever() || this->nextTick > startAt) {
			this->nextTick = startAt;
		}
	}
	this->nextProgress = UTime::now() + intervalBetweenProgress;
	this->targetPosition = newPosition;
	this->intermediateTargetPosition = newIntermediateTargetPosition;

	this->onProgress();
}

bool Motor::isActive()
{
	return !this->nextTick.isNever();
}

bool Motor::isMoving()
{
	return !this->nextTick.isNever() && (this->targetPosition != this->currentPosition || this->intermediateTargetPosition != this->targetPosition);
}

void Motor::tick()
{
	// On est arriv� � destination.
	long distance = abs((long)(intermediateTargetPosition - currentPosition)) + abs((long)(intermediateTargetPosition - targetPosition));
	int8_t dir = sgn(intermediateTargetPosition - currentPosition);
	if (dir == 0) {
		this->intermediateTargetPosition = this->targetPosition;
		dir = sgn(intermediateTargetPosition - currentPosition);
	}
	distance *= dir;

	if (distance == 0) {
		// On est arret�. Plus rien � faire.
		this->clearOutput();
		this->nextTick = UTime::never();
		this->onProgress();
	} else {
		if (this->speedLevel == 0) {
			// Commencer � 1...
			this->speedLevel = dir;
		} else {
			// Incrementation progressive
			if (this->speedLevel * dir < maxAccelStep) {
				this->speedLevel += dir;

			} else {
				// On est au maxi.. Il ne faut pas d�passer.
				this->speedLevel = maxAccelStep * dir;
			}
			if (this->speedLevel * dir >= distance * dir) {
				this->speedLevel = distance;
			}
		}
		this->currentPosition += sgn(this->speedLevel);
		if (this->currentPosition == this->intermediateTargetPosition) {
			this->intermediateTargetPosition = this->targetPosition;
		}
		this->setOutput(this->currentPosition & 7);

		// Now compute the tick duration
		if (distance - dir == 0) {
			this->speedLevel = 0;
			this->nextTick += 1000*((uint32_t)pauseAfterMove);

			this->nextProgress = UTime::now() + intervalBetweenProgress;

			this->onTargetPositionReached();
		} else {
			this->nextTick += getPulseDuration(this->speedLevel);
			if (UTime::now() > this->nextProgress) {
				this->nextProgress = UTime::now() + intervalBetweenProgress;
				this->onProgress();
			}
		}
	}
}

long Motor::getPulseDuration(int speedLevel4) {
	long mult = speedLevel4 > 0 ? this->fastestPerHalfStepAsc : this->fastestPerHalfStepDesc;

	speedLevel4 = abs(speedLevel4);
	if (speedLevel4 > maxAccelStep ) {
		speedLevel4 = maxAccelStep;
	}
	long result = ((long) mult * (long) maxAccelStep) / (1 + speedLevel4);

	return result;
}
