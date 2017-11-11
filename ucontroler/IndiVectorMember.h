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
	const __FlashStringHelper * name;
	const __FlashStringHelper * label;
	int value;
	int min, max;
	IndiVector * vector;
	IndiVectorMember * next;
public:
	IndiVectorMember(IndiVector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label,
			int min, int max);

	void setValue(int v);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* INDIVECTORMEMBER_H_ */
