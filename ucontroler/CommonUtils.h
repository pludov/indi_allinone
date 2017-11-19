#ifndef COMMONUTILS_H
#define COMMONUTILS_H 1

#ifdef ARDUINO
#define DEBUG1(A) { Serial1.print(A);   Serial1.write('\r');Serial1.write('\n'); }
#define DEBUG2(A, B) { Serial1.print(A);Serial1.print(B);   Serial1.write('\r');Serial1.write('\n'); }
#define DEBUG3(A, B, C) { Serial1.print(A);Serial1.print(B);Serial1.print(C);   Serial1.write('\r');Serial1.write('\n');}
#define DEBUG4(A, B, C, D) { Serial1.print(A);Serial1.print(B);Serial1.print(C);Serial1.print(D);   Serial1.write('\r');Serial1.write('\n');}
#else

#include <iostream>
#define DEBUG1(A) (std::cerr << A << "\n")
#define DEBUG2(A, B) (std::cerr << A << B << "\n")
#define DEBUG3(A, B, C) (std::cerr << A << B << C << "\n")
#define DEBUG4(A, B, C, D) (std::cerr << A << B << C << D <<"\n")

#define F(A) (A)
#endif

#define GET_DEBUG_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define DEBUG(...) GET_DEBUG_MACRO(__VA_ARGS__, DEBUG4, DEBUG3, DEBUG2, DEBUG1)(__VA_ARGS__)

#endif