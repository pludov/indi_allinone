#ifndef COMMONUTILS_H
#define COMMONUTILS_H 1

#ifdef ARDUINO
#ifdef __AVR_ATmega2560__
	void suicide();
	#define DEBUG(...)
	#define FATAL(...) suicide()
#else
	// #ifndef TEENSYDUINO
	// #error "unsupported build"
	// #endif

#ifdef TEENSYDUINO
#define SerialDbg Serial
#else
#define SerialDbg Serial1
#endif

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

void debugOutputLock();
void debugOutputUnlock();

#define DEBUG1(A) { debugOutputLock(); debugItem(A);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG2(A, B) { debugOutputLock(); debugItem(A);debugItem(B);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG3(A, B, C) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG4(A, B, C, D) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);debugItem(D);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG5(A, B, C, D, E) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG6(A, B, C, D, E, F) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);debugItem(F);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG7(A, B, C, D, E, F, G) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);debugItem(F);debugItem(G);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define DEBUG8(A, B, C, D, E, F, G, H) { debugOutputLock(); debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);debugItem(F);debugItem(G);debugItem(H);   SerialDbg.write('\r');SerialDbg.write('\n'); debugOutputUnlock(); }
#define GET_DEBUG_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG8, DEBUG7, DEBUG6, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)

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
#define DEBUG7(A, B, C, D, E, F, G) ((*debugStream) << A << B << C << D << E << F << G <<"\n")
#define DEBUG8(A, B, C, D, E, F, G, H) ((*debugStream) << A << B << C << D << E << F << G << H <<"\n")

#define F(A) (A)

#define GET_DEBUG_MACRO(_1,_2,_3,_4,_5,_6, _7, _8, NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG8, DEBUG7, DEBUG6, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)
#define FATAL(...) DEBUG(__VA_ARGS__)
#endif


#endif
