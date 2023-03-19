#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "CommonUtils.h"

#ifdef ARDUINO

#ifndef __AVR_ATmega2560__

template<>
void debugItem<const __FlashStringHelper*>(const __FlashStringHelper* t) {
	PGM_P p = reinterpret_cast<PGM_P>(t);

	while (1) {
		char c = pgm_read_byte(p++);
		if (c == 0) break;
		debugItem(c);
	}
};
#else

void suicide() {
	pinMode(13, OUTPUT);
	while(1) {
		for(int i = 0; i < 3; ++i) {
			digitalWrite(13,HIGH);
			delay(50);
			digitalWrite(13,LOW);
			delay(50);
		}
		delay(200);
	}
}

#endif
#else

std::ostream * debugStream = &std::cerr;

#endif
