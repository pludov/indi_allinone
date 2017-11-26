/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTOR_H_
#define INDIVECTOR_H_


#include "Symbol.h"
#include "WriteBuffer.h"

#define VECNONE ((uint8_t)-1)

#define VECTOR_WRITABLE 1
#define VECTOR_READABLE 2
#define VECTOR_BUSY 4
#define VECTOR_HIDDEN 8


// define/delete property
#define VECTOR_ANNOUNCED 0
// when a flag of the prop/vector change (busy, ro, ...)
#define VECTOR_MUTATION 1
// only when the value of a vector change
#define VECTOR_VALUE 2
#define VECTOR_COMM_COUNT 3

class IndiProtocol;
class IndiDevice;
class IndiVectorMember;
class IndiVectorGroup;
class IndiIntVectorMember;


#define VECTORKIND_NEED_MEMBER_SUBTYPE 1

typedef IndiVector * (*FuncNewVector)(Symbol name, Symbol label);
typedef IndiVectorMember * (*FuncNewMember)(IndiVector * vec, Symbol name, Symbol label, uint8_t subType);

struct VectorKind {
	// is uid required here ?
	Symbol defVectorText;
	Symbol newVectorText;
	Symbol oneMemberText;
	// IndiTextVectorKindUid, IndiNumberVectorKindUid, ...
	uint8_t uid;
	// VECTORKIND_NEED_MEMBER_SUBTYPE
	uint8_t flag;

	FuncNewVector vectorFactory;
	FuncNewMember memberFactory;

	/** true if member must be typed */
	bool hasMemberSubtype() const { return flag & VECTORKIND_NEED_MEMBER_SUBTYPE; };
	IndiVector * newVector(Symbol name, Symbol label) const {return (*vectorFactory)(name, label); };
	IndiVectorMember * newMember(IndiVector * parent, Symbol name, Symbol label, uint8_t subType) const {return (*memberFactory)(parent, name, label, subType); };
	
};

class ReadBuffer;

class IndiVectorUpdateRequest {
public:
	int updatedMemberCount;

	ReadBuffer * readBuffer;
	uint16_t endOffset;

	// Ptr to actually updated member
	IndiVectorMember * * members;

	// Offset of data within readBuffer
	uint16_t * offsets;

	IndiVectorUpdateRequest(ReadBuffer * rb, IndiVectorMember * * members, uint16_t * offsets);

	void addItem(IndiVectorMember * member, uint16_t offset);

	void seekAt(int itemId);

	void markEnd();
	void unseek();
};

class IndiVector {
	friend class IndiProtocol;
	friend class IndiDevice;
	friend class IndiVectorMember;
	friend class WriteBuffer;
	friend class XmlWriteBuffer;
	friend class BinSerialWriteBuffer;
public:
	Symbol name;
	int8_t nameSuffix;
	Symbol label;

	IndiVectorGroup * group;
	IndiVectorMember * first, * last;
	
	// VECTOR_READABLE, VECTOR_WRITABLE, VECTOR_BUSY or not
	uint8_t flag;

	// 2 bits for each writer : announced, updated
	uint8_t notifStatus[VECTOR_COMM_COUNT];
	
	uint8_t uid;

	/** 
	 * clientId : IndiProtocol id
	 * commId : VECTOR_ANNOUNCED, VECTOR_UPDATED, ...
	 */
	bool isDirty(uint8_t clientId, uint8_t commId);
	bool cleanDirty(uint8_t clientId, uint8_t commId);
	// True on change
	bool setDirty(uint8_t clientId, uint8_t commId);
	void resetClient(uint8_t clientId);
	void notifyUpdate(uint8_t commId);
	
	void dumpMembers(WriteBuffer & into);

	void sendDefinition(WriteBuffer & into);
public:
	IndiVector(IndiVectorGroup * parent, Symbol name, Symbol label, uint8_t initialFlag = VECTOR_READABLE, bool autoregister = true);
	virtual ~IndiVector();

	bool hidden() const {
		return flag & VECTOR_HIDDEN;
	}

	uint8_t getFlag() const {
		return flag;
	}
	
	void set(uint8_t flagToChange, bool status);
	
	int getMemberCount() const;

	virtual const VectorKind & kind() const = 0;
	
	/**
	 * Write the vector at the given commId
	 * (Higher means less information)
	 *
	 */
	virtual void sendAnnounce(WriteBuffer & into);

	virtual void sendMutation(WriteBuffer & into);

	virtual void sendValue(WriteBuffer & into);

	// True on change, false otherwise
	virtual bool doUpdate(IndiVectorUpdateRequest & request);
};


#endif /* INDIVECTOR_H_ */
