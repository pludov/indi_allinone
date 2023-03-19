#ifndef INDINUMBERVECTOR_H_
#define INDINUMBERVECTOR_H_


#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiNumberVectorKind;

class IndiNumberVector : public IndiVector {
public:
    IndiNumberVector(const Symbol & group, const Symbol & name, const Symbol & label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);

    virtual const VectorKind & kind() const;

	static IndiVector * vectorFactory(const Symbol & group, const Symbol & name, const Symbol & label);
	static IndiVectorMember * memberFactory(IndiVector * vector, const Symbol & name, const Symbol & label, uint8_t subType);
};

#endif
