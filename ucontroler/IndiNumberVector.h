#ifndef INDINUMBERVECTOR_H_
#define INDINUMBERVECTOR_H_


#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiNumberVectorKind;

class IndiNumberVector : public IndiVector {
public:
    IndiNumberVector(IndiVectorGroup * parent, Symbol name, Symbol label);

    virtual bool hasMemberSubtype() const;
	virtual const VectorKind & kind() const;
};

#endif
