#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_

#include <cstdint>
#include "Symbol.h"

// The request types
#define BINDI_REQ_ANNOUNCE_NUMBER_VECTOR 128 // Full definition + hidden/visible
#define BINDI_REQ_MUTATE_NUMBER_VECTOR 129  // All props except current value
#define BINDI_REQ_UPDATE_NUMBER_VECTOR 131	


// Les properties

// All vectors kinds
#define INDI_NUMBER_VECTOR 0


class IndiVector;

class WriteBuffer {
protected:
	char * ptr;
	int left;
	int totalSize;
public:
	WriteBuffer(char * into, int size);

	void append(char c);
	void append(const char * s);
	void append(Symbol s);

	void appendXmlEscaped(char c);

	void appendXmlEscaped(Symbol s);
	void appendXmlEscaped(const char * s);
	void appendSymbol(Symbol s, uint8_t suffix);
	bool finish();
	int size();
	bool isEmpty();


	virtual bool supportUpdateValue() const = 0;

	virtual void writeDeleteVectorPacket(const IndiVector & vec) = 0;
	
	virtual void startAnnounceVectorPacket(const IndiVector & vec) = 0;
	virtual void endAnnounceVectorPacket(const IndiVector & vec) = 0;
	
	virtual void startMutateVectorPacket(const IndiVector & vec) = 0;
	virtual void endMutateVectorPacket(const IndiVector & vec) = 0;

	virtual void writeVectorFlag(uint8_t flag) = 0;

	// during announce/mutate
	virtual void startMember(const IndiVector & vec) = 0;
	virtual void endMember(const IndiVector & vec) = 0;

	virtual void startUpdateValuesPacket(const IndiVector & vec) = 0;
	virtual void endUpdateValuesPacket(const IndiVector & vec) = 0;
	
	

	virtual void writeVectorName(Symbol name, uint8_t suffix) = 0;
	virtual void writeVectorLabel(Symbol name, uint8_t suffix) = 0;
	virtual void writeVectorUid(uint8_t uid) = 0;
	virtual void writeVectorMemberSubtype(uint8_t subtype) = 0;
	virtual void writeVectorMemberName(Symbol name, uint8_t suffix) = 0;
	virtual void writeVectorMemberLabel(Symbol name, uint8_t suffix) = 0;
	virtual void writeFloat(float value) = 0;
	virtual void writeInt(int32_t value) = 0;
	virtual void writeString(const char * c) = 0;

};

#endif /* WRITEBUFFER_H_ */
