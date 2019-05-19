#ifndef utils_H_
#define utils_H_
#include "define.h"

/* utility methods */
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);
int bindOnPort(int, uint16_t);

#endif