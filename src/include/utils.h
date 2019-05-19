#ifndef utils_H_
#define utils_H_
#include "define.h"

/* utility methods */
void usage_error();
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);

#endif