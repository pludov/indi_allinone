#ifndef INDITEXTVECTOR_H_
#define INDITEXTVECTOR_H_


#include "IndiVector.h"

class IndiTextVector : public IndiVector {
public:
    IndiTextVector(IndiVectorGroup * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);

    virtual void dump(WriteBuffer & into);
};

#endif
