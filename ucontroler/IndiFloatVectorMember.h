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

	virtual uint8_t getSubtype() const { return subType; };
	virtual void writeValue(WriteBuffer & into) const;

	static constexpr int subType = 1;
};

#endif /* INDIFLOATVECTORMEMBER_H_ */
