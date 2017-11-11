#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_


class WriteBuffer {
	char * ptr;
	int left;
	int totalSize;
public:
	WriteBuffer(char * into, int size);

	void append(char c);
	void append(const char * s);
	void append(const __FlashStringHelper * s);

	void appendXmlEscaped(char c);
	void appendXmlEscaped(const __FlashStringHelper * s);
	void appendXmlEscaped(const char * s);
	bool finish();
	int size();
};

#endif /* STATUS_H_ */
