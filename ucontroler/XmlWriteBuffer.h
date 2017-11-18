#ifndef XMLWRITEBUFFER_H_
#define XMLWRITEBUFFER_H_ 1

#include "WriteBuffer.h"


class XmlWriteBuffer : public WriteBuffer {

	void appendSymbol(Symbol s, uint8_t suffix);
	void append(const char * s);
	void append(Symbol s);
	void append(char c) { WriteBuffer::append(c); }
	void appendXmlEscaped(char c);

	void appendXmlEscaped(Symbol s);
	void appendXmlEscaped(const char * s);
	
public:
	XmlWriteBuffer(uint8_t * into, int size);

	virtual bool supportUpdateValue() const;

	virtual void startWelcomePacket();
	
	virtual void writeDeleteVectorPacket(const IndiVector & vec);
	
	virtual void startAnnounceVectorPacket(const IndiVector & vec);
	virtual void endAnnounceVectorPacket(const IndiVector & vec);
	
	virtual void startMutateVectorPacket(const IndiVector & vec);
	virtual void endMutateVectorPacket(const IndiVector & vec);

	virtual void writeVectorFlag(uint8_t flag);

	// during announce/mutate
	virtual void startMember(const IndiVector & vec);
	virtual void endMember(const IndiVector & vec);

	virtual void startUpdateValuesPacket(const IndiVector & vec);
	virtual void endUpdateValuesPacket(const IndiVector & vec);

	virtual void writeVectorName(Symbol name, uint8_t suffix);
	virtual void writeVectorLabel(Symbol name, uint8_t suffix);
	virtual void writeVectorUid(uint8_t uid);
	virtual void writeVectorMemberSubtype(uint8_t subtype);
	virtual void writeVectorMemberName(Symbol name, uint8_t suffix);
	virtual void writeVectorMemberLabel(Symbol name, uint8_t suffix);
	virtual void writeFloat(float value);
	virtual void writeInt(int32_t value);
	virtual void writeString(const char * c);

};


#endif
