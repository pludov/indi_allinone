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
public:
	IndiSwitchVector(IndiVectorGroup * parent, Symbol name, Symbol label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);
	virtual ~IndiSwitchVector();

	virtual const VectorKind & kind() const;

    virtual bool doUpdate(IndiVectorUpdateRequest & request);

	static IndiVector * vectorFactory(Symbol name, Symbol label);
	static IndiVectorMember * memberFactory(IndiVector * vector, Symbol name, Symbol label, uint8_t subType);
};

#endif /* INDISWITCHVECTOR_H_ */
