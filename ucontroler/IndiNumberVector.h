#ifndef INDINUMBERVECTOR_H_
#define INDINUMBERVECTOR_H_


#include "IndiVector.h"

class IndiNumberVector : public IndiVector {
public:
    IndiNumberVector(IndiVectorGroup * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);

    virtual void dump(WriteBuffer & into);
};

#endif
