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

	void notifyVectorUpdate(uint8_t commId);
public:
	IndiVectorMember(IndiVector * vector, 
			const __FlashStringHelper * name, 
			const __FlashStringHelper * label);
	virtual ~IndiVectorMember();
	
	virtual uint8_t getSubtype() const = 0;

	virtual void writeValue(WriteBuffer & into) const = 0;
};

#endif /* INDIVECTORMEMBER_H_ */
