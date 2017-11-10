/*
 * Status.cpp
 *
 *  Created on: 27 févr. 2015
 *      Author: utilisateur
 */
#include <Arduino.h>
#include "Variable.h"

// a 115200, on transmet un caractère (9 bits), en: 1000000 / (115200 / 9) us
#define CHAR_XMIT_DELAY 79


WriteBuffer::WriteBuffer(char * into, int size)
{
	this->ptr = into;
	this->left = size;
}

void WriteBuffer::append(char c)
{
	if (this->left) {
		this->ptr[0] = c;
		this->ptr++;
		this->left--;
	} else {
		this->left = -1;
	}
}

static char hex(uint8_t v)
{
	if (v < 10) {
		return '0' + v;
	}
	return 'a' - 10 + v;
}

void WriteBuffer::append(const char * s)
{
	append('"');
	while(*s) {
		unsigned char c = *(s++);
		switch (c) {
			case '\\':
			case '"':
			case '/':
				append('\\');
				append(c);
				break;
			case '\b':
				append('\\');
				append('b');
				break;
			case '\t':
				append('\\');
				append('t');
				break;
			case '\n':
				append('\\');
				append('n');
				break;
			case '\f':
				append('\\');
				append('f');
				break;
			case '\r':
				append('\\');
				append('i');
				break;
			default:
				if (c < ' ') {
					append('\\');
					append('u');
					append('0');
					append('0');
					append(hex(c >> 4));
					append(hex(c & 15));
				} else {
					append(c);
				}
		}
	}
	append('"');
}

bool WriteBuffer::finish()
{
	if (left) {
		*ptr = 0;
		return true;
	}
	return false;
}



NodeElement::NodeElement(const char * name)
{
	this->name = name;
}

NodeContainer::NodeContainer()
	: NodeElement(0)
{
	first = 0;
	last = 0;
}

void NodeContainer::dump(WriteBuffer & into)
{
	into.append('{');
	bool empty = true;
	for(NodeContained * cur = first; (cur); cur=cur->next)
	{
		into.append(cur->name);
		into.append(':');
		cur->dump(into);
		if(empty) {
			empty = false;
		} else {
			into.append(',');
		}
	}
	into.append('}');
}

NodeContained::NodeContained(NodeContainer * parent)
	: NodeElement(0)
{
	this->parent = parent;
	next = 0;
	if (parent) {
		prev = parent->last;

		if (parent->last) {
			parent->last->next = this;
		} else {
			parent->first = this;
		}
		parent->last = this;
	} else {
		prev = 0;
	}
}

Scope::Scope(Root * parent, const char * name)
	: NodeElement(name), NodeContained(parent)
{
}

void Scope::dump(WriteBuffer & into)
{
	NodeContainer::dump(into);
}

Root::Root()
	: NodeElement(0), NodeContainer()
{
}


Variable::Variable(Scope * parent, const char * name)
	: NodeElement(name), NodeContained(parent)
{

}


void Variable::dump(WriteBuffer & into)
{
	into.append('n');
	into.append('u');
	into.append('l');
	into.append('l');
}