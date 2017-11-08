/*
 * voltmeter.h
 *
 *  Created on: 7 févr. 2015
 *      Author: utilisateur
 */

#ifndef VOLTMETER_H_
#define VOLTMETER_H_

#include "Scheduled.h"

class Voltmeter : public Scheduled {
	uint8_t pin;
	// en centi volt
	unsigned int lastv;
public:
	Voltmeter(uint8_t pin);
	virtual ~Voltmeter();

	virtual void tick();

	float lastValue();
};

extern Voltmeter voltmeter;


#endif /* VOLTMETER_H_ */
