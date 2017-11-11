/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDITEXTVECTORMEMBER_H_
#define INDITEXTVECTORMEMBER_H_

#include "IndiVectorMember.h"

class IndiTextVector;

class IndiTextVectorMember : public IndiVectorMember {
	friend class IndiVector;
	char * value;
public:
	IndiTextVectorMember(IndiTextVector * vector,
			const __FlashStringHelper * name,
			const __FlashStringHelper * label,
			uint8_t maxSze);

	void setValue(const char * value);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* INDITEXTVECTORMEMBER_H_ */
