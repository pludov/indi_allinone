#ifndef INDINUMBERVECTOR_H_
#define INDINUMBERVECTOR_H_


#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiNumberVectorKind;

class IndiNumberVector : public IndiVector {
public:
    IndiNumberVector(IndiVectorGroup * parent, Symbol name, Symbol label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);

    virtual const VectorKind & kind() const;

    virtual void doUpdate(IndiVectorUpdateRequest & request);

	static IndiVector * vectorFactory(Symbol name, Symbol label);
	static IndiVectorMember * memberFactory(IndiVector * vector, Symbol name, Symbol label, uint8_t subType);
};

#endif
