#ifndef dbclientOperations_H_
#define dbclientOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"
#include "utils.h"
#include "circularBuffer.h"
#include "fd.h"

#define LOG_ON 0
#define LOG_OFF 1
#define USER_ON 0
#define USER_OFF 1
#define MIRROR "mirror"

struct clientResources{
    pthread_mutex_t bufferMutex;
    struct circularBuffer buffer;
    pthread_cond_t fullBuffer;
    pthread_cond_t emptyBuffer;
    pthread_mutex_t listMutex;
    struct G_list list;
};

//initialization
int informServer(uint8_t, int, struct sockaddr_in*);
int getClients(int, struct sockaddr_in*, struct clientResources*);

//request handling on main thread
int handleRequest(char*, char*, int, struct clientResources*, struct clientInfo*);
int handleGetFileList(int, char*);
int handleGetFile(int, char*);

//worker thread methods
void* dbclientWorker(void*);
int getFileList(int, struct clientResources*, struct fileInfo*);
int getFile(int, struct clientResources*, struct fileInfo*);


//general client operations
int addClient(struct fileInfo*, struct clientResources*);
int addTask(struct fileInfo*, struct clientResources*);
int removeClient(struct clientInfo*, struct clientResources*);
int rsrcInit(struct clientResources*, int);
int rsrcFree(struct clientResources*);
int confirmClient(struct clientInfo*, struct clientResources*);
int waitOnCondition(pthread_cond_t*, pthread_mutex_t*);
void powerOff();

//utilities for main thread
void handler(int);
void free_rsrc(int);
void usage_error(const char*);
void perror_free(char*);
#endif