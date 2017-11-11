/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTOR_H_
#define INDIVECTOR_H_



#include "IndiDevice.h"

#define VECNONE ((uint8_t)-1)

#define VECTOR_WRITABLE 1
#define VECTOR_READABLE 2
#define VECTOR_BUSY 4

#define VECTOR_ANNOUNCED 0
#define VECTOR_MUTATION 1
#define VECTOR_VALUE 2
#define VECTOR_COMM_COUNT 3

class IndiProtocol;
class IndiDevice;
class IndiVectorMember;
class IndiVectorGroup;
class IndiIntVectorMember;

class IndiVector {
	friend class IndiProtocol;
	friend class IndiDevice;
	friend class IndiVectorMember;
	friend class IndiIntVectorMember;
protected:
	const __FlashStringHelper * name;
	int8_t nameSuffix;
	const __FlashStringHelper * label;

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
public:
	IndiVector(IndiVectorGroup * parent, const __FlashStringHelper * name, const __FlashStringHelper * label);

	void set(uint8_t flag, bool status);
	
	virtual void dump(WriteBuffer & into) = 0;
};


#endif /* INDIVECTOR_H_ */
