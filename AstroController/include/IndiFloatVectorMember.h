/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIFLOATVECTORMEMBER_H_
#define INDIFLOATVECTORMEMBER_H_

#include "IndiVectorMember.h"
#include "IndiNumberVectorMember.h"
#include "Symbol.h"
#include "BinSerialProtocol.h"

class IndiNumberVector;

class IndiFloatVectorMember : public IndiNumberVectorMember {
	friend class IndiVector;
	double value;
public:
	IndiFloatVectorMember(IndiNumberVector * vector, 
			const Symbol & name,
			const Symbol & label,
			double min, double max, double step);

	void setValue(double v);

	virtual uint8_t getSubtype() const { return subType; };
	virtual void writeValue(WriteBuffer & into) const;
	virtual bool readValue(ReadBuffer & from);
	virtual void skipUpdateValue(ReadBuffer & from) const;
	virtual void writeUpdateValue(WriteBuffer & into, void * ptr) const;

	double getValue() const {
		return value;
	}

	virtual double getDoubleValue() const {
		return value;
	}

	static constexpr int subType = IndiNumberVectorMemberFloat;
};

#endif /* INDIFLOATVECTORMEMBER_H_ */
