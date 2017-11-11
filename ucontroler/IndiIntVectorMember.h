/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIINTVECTORMEMBER_H_
#define INDIINTVECTORMEMBER_H_

#include "IndiVectorMember.h"

class IndiNumberVector;

class IndiIntVectorMember : public IndiVectorMember {
	friend class IndiVector;
	int value;
	int min, max;
public:
	IndiIntVectorMember(IndiNumberVector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label,
			int min, int max);

	void setValue(int v);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* INDIINTVECTORMEMBER_H_ */
