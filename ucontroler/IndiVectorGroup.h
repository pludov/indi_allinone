/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef INDIVECTORGROUP_H_
#define INDIVECTORGROUP_H_

#include "Symbol.h"

class WriteBuffer;

class IndiVectorGroup {
	Symbol name;
	int8_t suffix;
public:
	IndiVectorGroup(Symbol name, int8_t suffix = 0);

	// void dumpXmlEncoded(WriteBuffer & into) const;
};


#endif /* INDIVECTORGROUP_H_ */
