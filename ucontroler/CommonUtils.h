#ifndef COMMONUTILS_H
#define COMMONUTILS_H 1

#ifdef ARDUINO
#ifdef __AVR_ATmega2560__
	void suicide();
	#define DEBUG(...)
	#define FATAL(...) suicide()
	#define MORE_DEBUG
#else
	#ifndef TEENSYDUINO
	#error "unsupported build"
	#endif


#define SerialDbg Serial

template<typename T>
void debugItem(T t) { SerialDbg.print(t); };

template<>
void debugItem<const __FlashStringHelper*>(const __FlashStringHelper* t);/* {
	PGM_P p = reinterpret_cast<PGM_P>(t);

	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		debugItem(c);
	}
};*/

#define DEBUG1(A) { debugItem(A);   SerialDbg.write('\r');SerialDbg.write('\n'); }
#define DEBUG2(A, B) { debugItem(A);debugItem(B);   SerialDbg.write('\r');SerialDbg.write('\n'); }
#define DEBUG3(A, B, C) { debugItem(A);debugItem(B);debugItem(C);   SerialDbg.write('\r');SerialDbg.write('\n');}
#define DEBUG4(A, B, C, D) { debugItem(A);debugItem(B);debugItem(C);debugItem(D);   SerialDbg.write('\r');SerialDbg.write('\n');}
#define DEBUG5(A, B, C, D, E) { debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);   SerialDbg.write('\r');SerialDbg.write('\n');}
#define DEBUG6(A, B, C, D, E, F) { debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);debugItem(F);   SerialDbg.write('\r');SerialDbg.write('\n');}

#define GET_DEBUG_MACRO(_1,_2,_3,_4,_5,_6, NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG6, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)

#define FATAL(...) DEBUG(__VA_ARGS__)
#endif
#else

#include <iostream>
extern std::ostream * debugStream;
#define DEBUG1(A) ((*debugStream) << A << "\n")
#define DEBUG2(A, B) ((*debugStream) << A << B << "\n")
#define DEBUG3(A, B, C) ((*debugStream) << A << B << C << "\n")
#define DEBUG4(A, B, C, D) ((*debugStream) << A << B << C << D <<"\n")
#define DEBUG5(A, B, C, D, E) ((*debugStream) << A << B << C << D << E <<"\n")
#define DEBUG6(A, B, C, D, E, F) ((*debugStream) << A << B << C << D << E << F <<"\n")

#define F(A) (A)

#define GET_DEBUG_MACRO(_1,_2,_3,_4,_5,_6, NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG6, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)
#define FATAL(...) DEBUG(__VA_ARGS__)
#endif


#endif
