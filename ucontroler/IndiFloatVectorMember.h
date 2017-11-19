/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIFLOATVECTORMEMBER_H_
#define INDIFLOATVECTORMEMBER_H_

#include "IndiVectorMember.h"
#include "Symbol.h"
#include "BinSerialProtocol.h"

class IndiNumberVector;

class IndiFloatVectorMember : public IndiVectorMember {
	friend class IndiVector;
	double value;
	double min, max;
public:
	IndiFloatVectorMember(IndiNumberVector * vector, 
			Symbol name, 
			Symbol label,
			double min, double max);

	void setValue(double v);

	virtual uint8_t getSubtype() const { return subType; };
	virtual void writeValue(WriteBuffer & into) const;
	virtual void readValue(ReadBuffer & from);

	static constexpr int subType = IndiNumberVectorMemberFloat;
};

#endif /* INDIFLOATVECTORMEMBER_H_ */
