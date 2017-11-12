#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_

#include <cstdint>

class WriteBuffer {
	char * ptr;
	int left;
	int totalSize;
public:
	WriteBuffer(char * into, int size);

	void append(char c);
	void append(const char * s);
#ifdef ARDUINO
	void append(const __FlashStringHelper * s);
#endif

	void appendXmlEscaped(char c);
#ifdef ARDUINO
	void appendXmlEscaped(const __FlashStringHelper * s);
#endif
	void appendXmlEscaped(const char * s);
	bool finish();
	int size();
};

#endif /* STATUS_H_ */
