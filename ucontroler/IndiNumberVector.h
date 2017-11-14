#ifndef INDINUMBERVECTOR_H_
#define INDINUMBERVECTOR_H_


#include "IndiVector.h"

extern const VectorKind IndiNumberVectorKind;

class IndiNumberVector : public IndiVector {
public:
    IndiNumberVector(IndiVectorGroup * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);

    virtual bool hasMemberSubtype() const;
	virtual const VectorKind & kind() const;
};

#endif
