/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDITEXTVECTORMEMBER_H_
#define INDITEXTVECTORMEMBER_H_

#include "IndiVectorMember.h"
#include "Symbol.h"

class IndiTextVector;

class IndiTextVectorMember : public IndiVectorMember {
	friend class IndiVector;
	char * value;
	uint8_t maxSize;
public:
	IndiTextVectorMember(IndiTextVector * vector,
			const Symbol & name,
			const Symbol & label,
			uint8_t maxSze);
	virtual ~IndiTextVectorMember();

	void setValue(const char * value);

	const char * getTextValue() const { return value; };

	virtual uint8_t getSubtype() const { return maxSize; };

	virtual void writeValue(WriteBuffer & into) const;
	virtual bool readValue(ReadBuffer & from);
	virtual void skipUpdateValue(ReadBuffer & from) const;
	virtual void writeUpdateValue(WriteBuffer & into, void * ptr) const;
};

#endif /* INDITEXTVECTORMEMBER_H_ */
