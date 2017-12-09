#ifndef COMMONUTILS_H
#define COMMONUTILS_H 1

#ifdef ARDUINO

/*#ifndef Serial1
#define Serial1 Serial
#endif*/

template<typename T>
void debugItem(T t) { Serial1.print(t); };

template<>
void debugItem<const __FlashStringHelper*>(const __FlashStringHelper* t);/* {
	PGM_P p = reinterpret_cast<PGM_P>(t);

	while (1) {
		unsigned char c = pgm_read_byte(p++);
		if (c == 0) break;
		debugItem(c);
	}
};*/

#define DEBUG1(A) { debugItem(A);   Serial1.write('\r');Serial1.write('\n'); }
#define DEBUG2(A, B) { debugItem(A);debugItem(B);   Serial1.write('\r');Serial1.write('\n'); }
#define DEBUG3(A, B, C) { debugItem(A);debugItem(B);debugItem(C);   Serial1.write('\r');Serial1.write('\n');}
#define DEBUG4(A, B, C, D) { debugItem(A);debugItem(B);debugItem(C);debugItem(D);   Serial1.write('\r');Serial1.write('\n');}
#define DEBUG5(A, B, C, D, E) { debugItem(A);debugItem(B);debugItem(C);debugItem(D);debugItem(E);   Serial1.write('\r');Serial1.write('\n');}
#else

#include <iostream>
#define DEBUG1(A) (std::cerr << A << "\n")
#define DEBUG2(A, B) (std::cerr << A << B << "\n")
#define DEBUG3(A, B, C) (std::cerr << A << B << C << "\n")
#define DEBUG4(A, B, C, D) (std::cerr << A << B << C << D <<"\n")
#define DEBUG4(A, B, C, D, E) (std::cerr << A << B << C << D << E <<"\n")
#define DEBUG5(A, B, C, D, E, F) (std::cerr << A << B << C << D << E << F <<"\n")

#define F(A) (A)
#endif

#define GET_DEBUG_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG5, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)

#endif
