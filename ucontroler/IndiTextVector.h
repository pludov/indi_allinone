#ifndef INDITEXTVECTOR_H_
#define INDITEXTVECTOR_H_


#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiTextVectorKind;

class IndiTextVector : public IndiVector {
public:
    IndiTextVector(IndiVectorGroup * parent, Symbol name, Symbol label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);
    
	virtual const VectorKind & kind() const;

	// FIXME: group !
	static IndiVector * vectorFactory(Symbol name, Symbol label);
	static IndiVectorMember * memberFactory(IndiVector * vector, Symbol name, Symbol label, uint8_t subType);

};

#endif
