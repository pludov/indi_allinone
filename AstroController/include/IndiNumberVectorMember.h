/*
 * IndiNumberVectorMember.h
 *
 *  Created on: 24 nov. 2017
 *      Author: ludovic
 */

#ifndef INDINUMBERVECTORMEMBER_H_
#define INDINUMBERVECTORMEMBER_H_

#include "IndiNumberVector.h"
#include "IndiVectorMember.h"

class IndiNumberVectorMember : public IndiVectorMember {
	double min, max, step;

public:
	IndiNumberVectorMember(IndiNumberVector * vector,
			const Symbol & name,
			const Symbol & label,
			double min, double max, double step);

	virtual ~IndiNumberVectorMember() = 0;

	virtual double getDoubleValue() const = 0;
};

#endif /* INDINUMBERVECTORMEMBER_H_ */
