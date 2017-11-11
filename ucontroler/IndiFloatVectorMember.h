/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIFLOATVECTORMEMBER_H_
#define INDIFLOATVECTORMEMBER_H_

#include "IndiVectorMember.h"

class IndiNumberVector;

class IndiFloatVectorMember : public IndiVectorMember {
	friend class IndiVector;
	double value;
	double min, max;
public:
	IndiFloatVectorMember(IndiNumberVector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label,
			double min, double max);

	void setValue(double v);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* INDIFLOATVECTORMEMBER_H_ */
