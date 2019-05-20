#ifndef dbclientOperations_H_
#define dbclientOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"
#include "utils.h"
#include "circularBuffer.h"

#define LOG_ON 0
#define LOG_OFF 1

struct clientResources{
    struct G_list list;
    struct circularBuffer buffer;
};


/*client operations*/
int informServer(uint8_t, int, struct sockaddr_in*);
int getClients(int, struct sockaddr_in*, struct clientResources*);
int handleRequest(char*, char*, int, struct clientResources*);
int sendFilePaths(int, char*);
int sendCertainFile(int, char*);
int addClient(int, struct clientResources*);
int removeClient(int, struct clientResources*);
int rsrcFree(struct clientResources*);
#endif