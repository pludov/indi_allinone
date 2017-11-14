/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTOR_H_
#define INDIVECTOR_H_


#include "Symbol.h"
#include "IndiDevice.h"

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


struct VectorKind {
	// is uid required here ?
	Symbol defVectorText;
	Symbol newVectorText;
	Symbol oneMemberText;
};

class IndiVector {
	friend class IndiProtocol;
	friend class IndiDevice;
	friend class IndiVectorMember;
	friend class WriteBuffer;
protected:
	Symbol name;
	int8_t nameSuffix;
	Symbol label;

	IndiVectorGroup * group;
	IndiVectorMember * first, * last;
	
	uint8_t flag;

	// 2 bits for each writer : announced, updated
	uint8_t notifStatus[VECTOR_COMM_COUNT];
	
	int8_t uid;

	/** 
	 * clientId : IndiProtocol id
	 * commId : VECTOR_ANNOUNCED, VECTOR_UPDATED, ...
	 */
	bool isDirty(uint8_t clientId, uint8_t commId);
	bool cleanDirty(uint8_t clientId, uint8_t commId);

	void notifyUpdate(uint8_t commId);
	
	void dumpMembers(WriteBuffer & into);

	void sendDefinition(WriteBuffer & into);
public:
	IndiVector(IndiVectorGroup * parent, Symbol name, Symbol label, uint8_t initialFlag = VECTOR_READABLE);

	bool hidden() const {
		return flag & VECTOR_HIDDEN;
	}

	uint8_t getFlag() const {
		return flag;
	}
	
	void set(uint8_t flagToChange, bool status);
	
	/** true if member must be typed */
	virtual bool hasMemberSubtype() const = 0;
	virtual const VectorKind & kind() const = 0;
	
	/**
	 * Write the vector at the given commId
	 * (Higher means less information)
	 *
	 */
	virtual void sendAnnounce(WriteBuffer & into);

	virtual void sendMutation(WriteBuffer & into);

	virtual void sendValue(WriteBuffer & into);
};


#endif /* INDIVECTOR_H_ */
