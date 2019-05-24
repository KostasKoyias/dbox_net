#include "../include/dbclient.h"
#include "../include/clientInfo.h"

// add a client to the list safely
int addClient(struct fileInfo* fileInfo, struct clientResources* rsrc){
    if(rsrc == NULL)
        return -1;

    // add client to client list
    pthread_mutex_lock(&(rsrc->listMutex));
    listInsert(&(rsrc->list), &(fileInfo->owner));
    listPrint(&(rsrc->list));
    pthread_mutex_unlock(&(rsrc->listMutex));
    return addTask(fileInfo, rsrc);
}

// add a task to the circularBuffer for a worker thread to complete
int addTask(struct fileInfo* fileInfo, struct clientResources* rsrc){
    if(rsrc == NULL)
        return -1;

    // add a GET_FILE or GET_FILE_LIST task to the circular buffer for a working thread to ask for all files of the new client 
    pthread_mutex_lock(&(rsrc->bufferMutex));
    while(bufferIsFull(&(rsrc->buffer)))
        waitOnCondition(&(rsrc->fullBuffer), &(rsrc->bufferMutex));
    bufferAdd(fileInfo, &(rsrc->buffer));
    bufferPrint(&(rsrc->buffer));

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
    listPrint(&(rsrc->list));
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
    
    // buffer resources
    pthread_mutex_destroy(&(rsrc->bufferMutex));
    pthread_cond_destroy(&(rsrc->fullBuffer));
    pthread_cond_destroy(&(rsrc->emptyBuffer));   
    bufferFree(&(rsrc->buffer));

    // client list resources
    pthread_mutex_destroy(&(rsrc->listMutex));
    listFree(&(rsrc->list));
    return 0;
}

// ensure that a peer trying to issue a request is really a member of the client list
int confirmClient(struct clientInfo* peerInfo, struct clientResources* rsrc){
    void* result;
    if(peerInfo == NULL || rsrc == NULL)
        return -1;

    // lock common resource "client list" and search in it for the (IP, port) pair of this client
    pthread_mutex_lock(&(rsrc->listMutex));
    result = listSearch(&(rsrc->list), peerInfo);
    pthread_mutex_unlock(&(rsrc->listMutex));
    return (result == NULL ? 0 : 1);
}


// wait until a condition is signaled/broadcasted, check whether that was because of a termination broadcast
int waitOnCondition(pthread_cond_t* cond, pthread_mutex_t* mutex){
    extern uint8_t powerOn;

    if(cond == NULL || mutex == NULL)
        return -1;

    // atomically unlock buffer and self-block until condition holds, then lock again as soon as possible
    pthread_cond_wait(cond, mutex); 
    
    // check whether this thread was signaled in order to log out, if so unlock mutex for other threads to exit as well
    if(!powerOn){
        pthread_mutex_unlock(mutex);
        powerOff();
    }
    return 0;
}

// terminate thread
void powerOff(){
    fprintf(stdout, "thread %ld: got power off!\n", pthread_self());
    pthread_exit(NULL);
}
