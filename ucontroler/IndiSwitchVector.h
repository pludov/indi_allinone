/*
 * IndiSwitchVector.h
 *
 *  Created on: 26 nov. 2017
 *      Author: ludovic
 */

#ifndef INDISWITCHVECTOR_H_
#define INDISWITCHVECTOR_H_

#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiSwitchVectorKind;

class IndiSwitchVectorMember;

class IndiSwitchVector : public IndiVector {
	friend class IndiSwitchVectorMember;
	IndiSwitchVectorMember * activeOne;
	void refreshActiveOne(IndiSwitchVectorMember * lastUpdated);
	bool independantMembers() const { return !!(flag & VECTOR_SWITCH_MANY); }
public:
	IndiSwitchVector(const Symbol & group, const Symbol & name, const Symbol & label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);
	virtual ~IndiSwitchVector();

	virtual const VectorKind & kind() const;

    virtual bool doUpdate(IndiVectorUpdateRequest & request);

    IndiSwitchVectorMember * getCurrent() const { return activeOne; };

	uint32_t getValueMask() const;

	static IndiVector * vectorFactory(const Symbol & group, const Symbol & name, const Symbol & label);
	static IndiVectorMember * memberFactory(IndiVector * vector, const Symbol & name, const Symbol & label, uint8_t subType);
};

#endif /* INDISWITCHVECTOR_H_ */
