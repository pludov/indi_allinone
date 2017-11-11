/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORMEMBER_H_
#define INDIVECTORMEMBER_H_

class IndiVectorMember {
	friend class IndiVector;
	IndiVectorMember * next;
protected:
	const __FlashStringHelper * name;
	const __FlashStringHelper * label;
	IndiVector * vector;
public:
	IndiVectorMember(IndiVector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix) = 0;
};

#endif /* INDIVECTORMEMBER_H_ */
