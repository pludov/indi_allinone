/*
 * Motor.h
 *
 *  Created on: 3 févr. 2015
 *      Author: utilisateur
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include "Scheduled.h"

class Motor : public Scheduled
{
protected:
	const uint8_t * motorPins;
	uint8_t positionBase;							   // Gets added to every position (for calibration support)
	unsigned long currentPosition;             // current position
	unsigned long targetPosition;              // target position
	unsigned long intermediateTargetPosition;  // Will pass there before reaching targetPosition (backlash)

	// Invert the rotation on the motor side.
	bool invert;

	// At full speed, how short a step is (us) ?
	int fastestPerHalfStepAsc;
	int fastestPerHalfStepDesc;
	// Number of steps to gain full speed
	uint8_t maxAccelStep = 100;
	// Pause after a move (ms), before cutting off signal and reporting idle
	uint16_t pauseAfterMove = 250;

	// Direction & speed
	int speedLevel;

	UTime nextProgress;
public:
	Motor(const uint8_t * pins, const Symbol & debug, int fastestPerHalfStepDesc = 4 * 2200, int fastestPerHalfStepAsc = 4 * 2200);

	bool getInvert() const { return invert; }
	void setInvert(bool b) { this->invert = b; }

	void updatePulse(int fastestPerHalfStepAsc, int fastestPerHalfStepDesc) {
		this->fastestPerHalfStepAsc = fastestPerHalfStepAsc;
		this->fastestPerHalfStepDesc = fastestPerHalfStepDesc;
	}

	int getFastestPerHalfStepAsc() const { return this->fastestPerHalfStepAsc; }
	int getFastestPerHalfStepDesc() const { return this->fastestPerHalfStepDesc; }

	// Load stored position
	void loadPosition(unsigned long currentPosition);
	void loadPosition(unsigned long currentPosition, uint8_t positionBase);

	// Start moving to the given position
	void setTargetPosition(unsigned long newPosition);
	// Start moving to the given position, with an intermediate step (for backlash)
	void setTargetPosition(unsigned long newPosition, unsigned long newIntermediatePosition);

	unsigned long getTargetPosition();

	unsigned long getCurrentPosition();

	// Is there a signal currently delivered ?
	bool isActive();

	// Is there an ongoing move ?
	bool isMoving();

	// Call when micros >= nextTick
	virtual void tick();

	// Call when currentPositoin == targetPosition during move (motor still not off)
	virtual void onTargetPositionReached() = 0;

	// Call during move, every ~500ms
	virtual void onProgress() = 0;
protected:
	void setMaxAccelStep(int m) { this->maxAccelStep = m; }
	uint8_t getMaxAccelStep() const { return this->maxAccelStep; }
	void setOutput(int out);
	void clearOutput();
	long getPulseDuration(int speedLevel);
};

#endif /* MOTOR_H_ */
