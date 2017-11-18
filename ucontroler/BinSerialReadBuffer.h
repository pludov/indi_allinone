#ifndef BINSERIALREADBUFFER_H_
#define BINSERIALREADBUFFER_H_ 1

class BinSerialReadBuffer {
    char * buffer;
    int ptr;
    int size;

public:
    void readAndApply();

}

#endif