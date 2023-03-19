#ifndef INDISYMBOLS_H_
#define INDISYMBOLS_H_

#include <stdint.h>

#ifdef ARDUINO

class Symbol {
public:
	const __FlashStringHelper * base;
	uint8_t suffix;

	Symbol(const __FlashStringHelper * base)
	{
		this->base = base;
		this->suffix = 0;
	}

	Symbol(const __FlashStringHelper * base, uint8_t suffix)
	{
		this->base = base;
		this->suffix = suffix;
	}

	operator String () const {
		return String(base);
	}
};

#else

#include <string>
class Symbol : public std::string {
public:
	Symbol(const char * value) : std::string(value) {}
	Symbol(const char * value, uint8_t suffix) : std::string(value) {
		if (suffix) {
			*this += '_';
			*this += '0' + suffix;
		}
	}
};

#endif

#endif /* INDISYMBOLS */
