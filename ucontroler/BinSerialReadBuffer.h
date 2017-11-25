#ifndef BINSERIALREADBUFFER_H_
#define BINSERIALREADBUFFER_H_ 1

#include <setjmp.h>
#include "Symbol.h"
#include "WriteBuffer.h"
#include "ReadBuffer.h"
#include "BinSerialWriteBuffer.h"



class IndiDevice;
class IndiProtocol;

class BinSerialReadBuffer : public ReadBuffer{
protected:
    // safe to call fail from here
    virtual void internalReadAndApply(IndiDevice & applyTo, IndiProtocol &proto, BinSerialWriteBuffer & answer);
    
    uint8_t readUint7();
    uint8_t readStringChar();
    uint8_t readUid();
    uint8_t readPacketControl();
    bool isAtEnd();
    void readSymbol(char * buffer, int maxLength);
    void skipSymbol(int maxLength);
    
    
public:
    BinSerialReadBuffer(uint8_t * buffer, int size);

    virtual float readFloat();
    virtual int32_t readInt();
    virtual void readString(char * buffer, int maxSize);
    virtual void skipString(int maxSize);

    void debug();
};

#endif
