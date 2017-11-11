/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORMEMBER
#define INDIVECTORMEMBER

class Member {
	friend class Vector;
	const __FlashStringHelper * name;
	const __FlashStringHelper * label;
	int value;
	int min, max;
	Vector * vector;
	Member * next;
public:
	Member(Vector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label,
			int min, int max);

	void setValue(int v);

	virtual void dump(WriteBuffer & into, int8_t nameSuffix);
};

#endif /* INDIVECTORMEMBER_H_ */
