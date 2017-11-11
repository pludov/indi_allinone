/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORGROUP_H_
#define INDIVECTORGROUP_H_


class Group {
	friend class Vector;
	const __FlashStringHelper * name;
public:
	Group(const __FlashStringHelper * name);
};


#endif /* INDIVECTORGROUP_H_ */
