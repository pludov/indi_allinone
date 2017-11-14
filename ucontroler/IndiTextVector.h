#ifndef INDITEXTVECTOR_H_
#define INDITEXTVECTOR_H_


#include "IndiVector.h"

extern const VectorKind IndiTextVectorKind;

class IndiTextVector : public IndiVector {
public:
    IndiTextVector(IndiVectorGroup * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);
    
    virtual bool hasMemberSubtype() const;
	virtual const VectorKind & kind() const;
};

#endif
