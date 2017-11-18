#ifndef INDITEXTVECTOR_H_
#define INDITEXTVECTOR_H_


#include "IndiVector.h"
#include "Symbol.h"

extern const VectorKind IndiTextVectorKind;

class IndiTextVector : public IndiVector {
public:
    IndiTextVector(IndiVectorGroup * parent, Symbol name, Symbol label);
    
    virtual bool hasMemberSubtype() const;
	virtual const VectorKind & kind() const;
};

#endif
