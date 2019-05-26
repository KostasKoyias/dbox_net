#ifndef utils_H_
#define utils_H_
#include "define.h"
#include "clientInfo.h"

// generic utility methods
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
int exclusive_print(int, const char*, ...);

// networking utility methods
int connectTo(struct sockaddr_in*);
int getListeningSocket(uint32_t, uint16_t);
int bindOnPort(int, uint16_t);
int getMyIp(struct in_addr*);
int getReuseAddrSocket();
int addrToInfo(struct sockaddr_in*, struct clientInfo*);
int infoToAddr(struct clientInfo*, struct sockaddr_in*);
int getIpOf(char*, struct in_addr*);


// other
int setPath(char*, const char*, const char*);
int lastIndexOf(char, const char*);
int makeParents(char*, mode_t);
int mkdirTree(char*, mode_t);
int statFile(char*);


#endif