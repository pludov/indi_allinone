/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIINTVECTORMEMBER_H_
#define INDIINTVECTORMEMBER_H_

#include <stdint.h>

#include "IndiVectorMember.h"
#include "IndiNumberVectorMember.h"
#include "Symbol.h"
#include "BinSerialProtocol.h"

class IndiNumberVector;


class IndiIntVectorMember : public IndiNumberVectorMember {
	friend class IndiVector;
	int32_t value;
public:
	IndiIntVectorMember(IndiNumberVector * vector, 
			const Symbol & name,
			const Symbol & label,
			int32_t min, int32_t max, int32_t step);

	void setValue(int32_t v);

	int32_t getValue() const { return value; }
	virtual double getDoubleValue() const { return value; }

	virtual uint8_t getSubtype() const { return subType; }
	virtual void writeValue(WriteBuffer & into) const;
	virtual bool readValue(ReadBuffer & from);
	virtual void skipUpdateValue(ReadBuffer & from) const;
	virtual void writeUpdateValue(WriteBuffer & into, void * ptr) const;

	static constexpr int subType = IndiNumberVectorMemberInt;
};

#endif /* INDIINTVECTORMEMBER_H_ */
