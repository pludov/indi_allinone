/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIINTVECTORMEMBER_H_
#define INDIINTVECTORMEMBER_H_

#include "IndiVectorMember.h"
#include "Symbol.h"
#include "BinSerialProtocol.h"

class IndiNumberVector;


class IndiIntVectorMember : public IndiVectorMember {
	friend class IndiVector;
	int32_t value;
	int32_t min, max;
public:
	IndiIntVectorMember(IndiNumberVector * vector, 
			Symbol name, 
			Symbol label,
			int32_t min, int32_t max);

	void setValue(int32_t v);

	virtual uint8_t getSubtype() const { return subType; }
	virtual void writeValue(WriteBuffer & into) const;
	virtual void readValue(ReadBuffer & from);

	static constexpr int subType = IndiNumberVectorMemberInt;
};

#endif /* INDIINTVECTORMEMBER_H_ */
