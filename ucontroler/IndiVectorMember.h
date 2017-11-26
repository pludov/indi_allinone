/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORMEMBER_H_
#define INDIVECTORMEMBER_H_

#include "Symbol.h"
#include "ReadBuffer.h"

class IndiVectorMember {
	friend class IndiVector;
public:
	IndiVectorMember * next;
	Symbol name;
	Symbol label;
	IndiVector * vector;

	void notifyVectorUpdate(uint8_t commId);
public:
	IndiVectorMember(IndiVector * vector, 
			Symbol name, 
			Symbol label);
	virtual ~IndiVectorMember();
	
	virtual uint8_t getSubtype() const = 0;

	virtual void writeValue(WriteBuffer & into) const = 0;
	// true when value changed. May or may not dirty the vector (depends on subclass)
	virtual bool readValue(ReadBuffer & from) = 0;
	virtual void skipUpdateValue(ReadBuffer & from) const = 0;
	virtual void writeUpdateValue(WriteBuffer & into, void * ptr) const = 0;
};

#endif /* INDIVECTORMEMBER_H_ */
