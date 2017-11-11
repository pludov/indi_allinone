/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORGROUP_H_
#define INDIVECTORGROUP_H_


class IndiVectorGroup {
	friend class IndiVector;
	friend class IndiNumberVector;
	const __FlashStringHelper * name;
public:
	IndiVectorGroup(const __FlashStringHelper * name);
};


#endif /* INDIVECTORGROUP_H_ */
