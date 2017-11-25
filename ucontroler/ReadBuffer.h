#ifndef READBUFFER_H_
#define READBUFFER_H_

#include <setjmp.h>
#include "Symbol.h"
#include "WriteBuffer.h"
#include "BinSerialWriteBuffer.h"

#ifndef ARDUINO
#include <string>
#endif

class IndiDevice;
class IndiProtocol;

class ReadBuffer {
	friend class IndiVectorUpdateRequest;
    jmp_buf parsePoint;
protected:
    uint8_t * buffer;
    uint8_t * bufferOrg;
    uint16_t left;

    [[ noreturn ]] void fail(Symbol s);
    // safe to call fail from here
    virtual void internalReadAndApply(IndiDevice & applyTo, IndiProtocol &proto, BinSerialWriteBuffer & answer) = 0;
    
	uint8_t readOne();
    uint8_t peekOne();

#ifndef ARDUINO
    std::string getError();
#endif
    
public:
    ReadBuffer(uint8_t * buffer, int size);
    // false indicate problem with data
    bool readAndApply(IndiDevice & applyTo, IndiProtocol & proto, BinSerialWriteBuffer & answer);

    void seekAt(uint16_t ptr);
    uint16_t getCurrentPos() const;

    virtual float readFloat() = 0;
    virtual int32_t readInt() = 0;
    virtual void readString(char * buffer, int maxSize) = 0;
    virtual void skipString(int maxSize) = 0;
};

#endif /* WRITEBUFFER_H_ */
