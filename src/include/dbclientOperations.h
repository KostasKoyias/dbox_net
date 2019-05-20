#ifndef dbclientOperations_H_
#define dbclientOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"
#include "utils.h"
#include "circularBuffer.h"

#define LOG_ON 0
#define LOG_OFF 1
#define USER_ON 0
#define USER_OFF 1
#define FILE_CODE_LEN 20

struct clientResources{
    pthread_mutex_t bufferMutex;
    struct circularBuffer buffer;
    pthread_cond_t fullBuffer;
    pthread_cond_t emptyBuffer;
    pthread_mutex_t listMutex;
    struct G_list list;
};


/*client operations*/
int informServer(uint8_t, int, struct sockaddr_in*);
int getClients(int, struct sockaddr_in*, struct clientResources*);
int handleRequest(char*, char*, int, struct clientResources*);
int handleGetFileList(int, char*);
int handleGetFile(int, char*);
int handleUser(int, int, struct clientResources*);
int sendFile(int, char*);
int addClient(struct clientInfo*, struct clientResources*);
int removeClient(struct clientInfo*, struct clientResources*);
int rsrcFree(struct clientResources*);
int confirmClient(struct sockaddr_in*, struct clientResources*);
#endif