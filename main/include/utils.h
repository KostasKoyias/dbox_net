#ifndef utils_H_
#define utils_H_
#include "define.h"

// generic utility methods
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);

// networking utility methods
int getListeningSocket(int);
int bindOnPort(int, uint16_t);
int getRequest(char*, char**, int, int, struct sockaddr_in*, socklen_t*);
int statFile(char*);
int establishConnection(struct sockaddr_in*, struct sockaddr_in*);


#endif