/*
 * IndiNumberVectorMember.cpp
 *
 *  Created on: 24 nov. 2017
 *      Author: ludovic
 */
#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "IndiNumberVectorMember.h"

IndiNumberVectorMember::IndiNumberVectorMember(IndiNumberVector * vector,
		const Symbol & name,
		const Symbol & label,
		double min, double max, double step)
	: IndiVectorMember(vector, name, label)
{
	// TODO Auto-generated constructor stub
	this->min = min;
	this->max = max;
	this->step = step;
}

IndiNumberVectorMember::~IndiNumberVectorMember() {
	// TODO Auto-generated destructor stub
}

