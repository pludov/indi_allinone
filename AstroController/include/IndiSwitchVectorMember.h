/*
 * IndiSwitchVectorMember.h
 *
 *  Created on: 26 nov. 2017
 *      Author: ludovic
 */

#ifndef INDISWITCHVECTORMEMBER_H_
#define INDISWITCHVECTORMEMBER_H_

#include "IndiVectorMember.h"
#include "Symbol.h"

class IndiSwitchVector;

class IndiSwitchVectorMember: public IndiVectorMember {
	friend class IndiSwitchVector;
	bool value;
public:
	IndiSwitchVectorMember(IndiSwitchVector * vector,
				const Symbol & name,
				const Symbol & label);
	virtual ~IndiSwitchVectorMember();

	void setValue(bool on);
	bool getValue() const { return value; };

	virtual uint8_t getSubtype() const { return 0; };

	virtual void writeValue(WriteBuffer & into) const;
	virtual bool readValue(ReadBuffer & from);
	virtual void skipUpdateValue(ReadBuffer & from) const;
	virtual void writeUpdateValue(WriteBuffer & into, void * ptr) const;
};

#endif /* INDISWITCHVECTORMEMBER_H_ */
