/*
 * IndiVectorStorage.h
 *
 *  Created on: 9 déc. 2017
 *      Author: ludovic
 */

#ifndef INDIVECTORMEMBERSTORAGE_H_
#define INDIVECTORMEMBERSTORAGE_H_

#include <stdint.h>

class IndiVectorMember;
class IndiFloatVectorMember;
class IndiTextVectorMember;
class IndiIntVectorMember;

class IndiVectorMemberStorage {
public:
	IndiVectorMemberStorage();
	virtual ~IndiVectorMemberStorage() = 0;

	virtual void save() = 0;

	static void remember(IndiFloatVectorMember * member, uint32_t addr);
	static void remember(IndiTextVectorMember * member, uint32_t addr);
	static void remember(IndiIntVectorMember * member, uint32_t addr);
};

#endif /* INDIVECTORMEMBERSTORAGE_H_ */
