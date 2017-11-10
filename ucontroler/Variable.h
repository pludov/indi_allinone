/*
 * status.h
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */

#ifndef VARIABLE_H_
#define VARIABLE_H_

#include "Scheduled.h"

class WriteBuffer {
	char * ptr;
	int left;
public:
	WriteBuffer(char * into, int size);

	void append(char c);
	void append(const char * s);

	bool finish();
};

class NodeElement {
protected:
	const char * name;
	NodeElement(const char * name);

public:
	virtual void dump(WriteBuffer & into) = 0;
};

class NodeContained;

class NodeContainer : virtual public NodeElement {
	friend class NodeContained;
protected:
	NodeContained * first, * last;

	NodeContainer();
public:
	virtual void dump(WriteBuffer & into);
};

class NodeContained : virtual public NodeElement {
	friend class NodeContainer;
protected:
	NodeContainer * parent;
	NodeContained * prev, * next;
	
	NodeContained(NodeContainer * parent);
public:
	virtual void dump(WriteBuffer & into) = 0;
};


class Root : public NodeContainer {
public:
	Root();
};

class Scope : public NodeContainer, public NodeContained {
public:
	Scope(Root * parent, const char * name);
	virtual void dump(WriteBuffer & into);
	
};

class Variable : public NodeContained {
	int8_t announced;
	int8_t updated;
	int8_t uid;
public:
	Variable(Scope * parent, const char * name);

	virtual void dump(WriteBuffer & into);
};

#endif /* STATUS_H_ */
