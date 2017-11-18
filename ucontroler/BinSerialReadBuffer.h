#ifndef BINSERIALREADBUFFER_H_
#define BINSERIALREADBUFFER_H_ 1

#include <csetjmp>
#include "Symbol.h"

#ifndef ARDUINO
#include <string>
#endif

class IndiDevice;

class BinSerialReadBuffer {
    std::jmp_buf parsePoint;
protected:
    uint8_t * buffer;
    int ptr;
    int left;

    [[ noreturn ]] void fail(Symbol s);
    // safe to call fail from here
    void internalReadAndApply(IndiDevice & applyTo);
    
    uint8_t readUint7();
    uint8_t readOne();
    uint8_t peekOne();
    uint8_t readStringChar();
    uint8_t readUid();
    uint8_t readPacketControl();
    bool isAtEnd();
    void readSymbol(char * buffer, int maxLength);
    float readFloat();
    int32_t readInt();
    
#ifndef ARDUINO
    std::string getError();
#endif
    
public:
    BinSerialReadBuffer(uint8_t * buffer, int size);
    // false indicate problem with data
    bool readAndApply(IndiDevice & applyTo);
};

#endif