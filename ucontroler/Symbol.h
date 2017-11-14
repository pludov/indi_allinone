#ifndef INDISYMBOLS_H_
#define INDISYMBOLS_H_

#include <cstdint>

#ifdef ARDUINO

typedef const __FlashStringHelper * Symbol;

#else

#include <string>
#define Symbol std::string

#endif

#endif /* INDISYMBOLS */
