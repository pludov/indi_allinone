#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "CommonUtils.h"

#ifdef ARDUINO
template<>
void debugItem<const __FlashStringHelper*>(const __FlashStringHelper* t) {
	PGM_P p = reinterpret_cast<PGM_P>(t);

	while (1) {
		char c = pgm_read_byte(p++);
		if (c == 0) break;
		debugItem(c);
	}
};

#endif
