#ifndef utils_H_
#define utils_H_
#include "define.h"

// generic utility methods
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);

// networking utility methods
int establishConnection(struct sockaddr_in*, struct sockaddr_in*);
int getListeningSocket(uint32_t, uint16_t);
int bindOnPort(int, uint16_t);
int getMyIp(struct in_addr*);
int statFile(char*);
int getReuseAddrSocket(struct sockaddr_in*);


#endif