#include "../include/dbclientOperations.h"
#include "../include/clientInfo.h"

// add a client to the list safely
int addClient(struct clientInfo* clientInfo, struct clientResources* rsrc){
    struct fileInfo fileInfo = {.path = "\0", .version = -1}; //version -1 indicates that this is a GET_FILE_LIST task
    if(rsrc == NULL)
        return -1;

    // add client to client list
    pthread_mutex_lock(&(rsrc->listMutex));
    listInsert(&(rsrc->list), clientInfo);
    pthread_mutex_unlock(&(rsrc->listMutex));

    // add GET_FILE_LIST task to the circular buffer for a working thread to ask for all files of the new client 
    clientAssign(&(fileInfo.owner), clientInfo);
    pthread_mutex_lock(&(rsrc->bufferMutex));
    while(bufferIsFull(&(rsrc->buffer))){
        pthread_mutex_unlock(&(rsrc->bufferMutex));
        pthread_cond_wait(&(rsrc->fullBuffer), &(rsrc->bufferMutex));
    }
    bufferAdd(&fileInfo, &(rsrc->buffer));

    pthread_cond_signal(&(rsrc->emptyBuffer));  // signal a pending worker thread
    pthread_mutex_unlock(&(rsrc->bufferMutex));
    return 0;
}

// handle a "USER_OFF" send by server
int removeClient(struct clientInfo* clientInfo,struct clientResources* rsrc){
    if(rsrc == NULL)
        return -1;

    // remove client from client list
    pthread_mutex_lock(&(rsrc->listMutex));
    listDelete(&(rsrc->list), clientInfo);
    pthread_mutex_unlock(&(rsrc->listMutex));
    return 0;
}

// allocate space for all resources used by this client
int rsrcInit(struct clientResources* rsrc, int bufferSize){
    if(rsrc == NULL)
        return -1;

    // based on the size just received by the server, initialize buffer to hold information for all files
    if(bufferInit(bufferSize, &(rsrc->buffer)) < 0)
        return -3;
    pthread_mutex_init(&(rsrc->bufferMutex), NULL);
    pthread_mutex_init(&(rsrc->listMutex), NULL);
    pthread_cond_init(&(rsrc->fullBuffer), NULL);
    pthread_cond_init(&(rsrc->emptyBuffer), NULL);
    return 0;
}

// free all space allocated for the client to operate
int rsrcFree(struct clientResources* rsrc){
    if(rsrc == NULL)
        return -1;
    
    bufferFree(&(rsrc->buffer));
    listFree(&(rsrc->list));
    return 0;
}

// ensure that a peer trying to issue a request is really a member of the client list
int confirmClient(struct sockaddr_in* peer, struct clientResources* rsrc){
    struct clientInfo peerInfo;
    void* result;
    if(peer == NULL || rsrc == NULL)
        return -1;

    // get peer info in the appropriate format in order to apply listSearch    
    peerInfo.ipAddress = peer->sin_addr.s_addr;
    peerInfo.portNumber = peer->sin_port;

    // lock common resource "client list" and search in it for the (IP, port) pair of this client
    pthread_mutex_lock(&(rsrc->listMutex));
    result = listSearch(&(rsrc->list), &peerInfo);
    pthread_mutex_unlock(&(rsrc->listMutex));
    return (result == NULL ? 0 : 1);
}
