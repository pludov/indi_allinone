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
	virtual ~IndiTextVectorMember();

	void setValue(const char * value);

	virtual uint8_t getSubtype() const { return 0; };

	virtual void writeValue(WriteBuffer & into) const;
};

#endif /* INDITEXTVECTORMEMBER_H_ */
