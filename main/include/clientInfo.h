#ifndef clientInfo_H_
#define clientInfo_H_
#include "define.h"

// for each client, hold a (ip, port) pair
struct clientInfo{
    uint32_t ipAddress;
    uint16_t portNumber;
};

// client linked list member methods
void* clientCompare(void*, const void*);
int clientAssign(void*, const void*);
int clientPrint(void*);
int getClientInfo(int, struct clientInfo*);
int sendClientInfo(int, struct clientInfo*);

#endif