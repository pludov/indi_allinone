#ifndef READBUFFER_H_
#define READBUFFER_H_

#include <cstdint>

class IndiVector;

class ReadBuffer {
	uint8_t * ptr;
	int left;
	int totalSize;
public:
	ReadBuffer(uint8_t * into, int size);

    

	virtual void writeDeleteVectorPacket(const IndiVector  & vec);
	virtual void startAnnounceVectorPacket(const IndiVector  & vec);
};

#endif /* WRITEBUFFER_H_ */
