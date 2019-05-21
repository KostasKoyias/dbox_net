#include "../include/dbclientOperations.h"

void* dbclientWorker(void* arg){
    struct clientResources* rsrc;
    struct fileInfo task;
    struct sockaddr_in peer = {.sin_family = AF_INET};
    int sock, rv;

    if(arg == NULL)
        return NULL;
    rsrc = (struct clientResources*)arg;

    while(1){

        // block until buffer is non-empty, then remove a task from circularBuffer
        pthread_mutex_lock(&(rsrc->bufferMutex));
        while(bufferIsEmpty(&(rsrc->buffer))){
            pthread_cond_wait(&(rsrc->emptyBuffer), &(rsrc->bufferMutex));
            pthread_mutex_lock(&(rsrc->bufferMutex));        
        }
        bufferRemove(&task, &(rsrc->buffer));
        pthread_cond_signal(&(rsrc->fullBuffer));   // signal a pending thread
        pthread_mutex_unlock(&(rsrc->bufferMutex));

        // create socket 
        if((sock = socket(AF_INET , SOCK_STREAM , 0)) == -1){
            perror("dbclient: working thread failed to create socket");
            continue;
        }

        //establish connection with peer
        peer.sin_addr.s_addr = task.owner.ipAddress;
        peer.sin_port = task.owner.portNumber;
        if(connect(sock, (struct sockaddr*)&peer, sizeof(peer)) == -1){
            perror("dbclient: working thread failed to connect");
            continue;
        }

        // if task is about syncing with a new user, i.e GET_FILE_LIST then version is set to -1, else it is about getting a certain file version from another client
        if(task.version == -1)
            rv = getFileList(sock, rsrc, &task);
        else
            rv = getFile(sock, rsrc, &task);
        if(rv < 0)
            perror("dbclient: working thread failed to complete task");
    }
    return NULL;
}

int getFileList(int socket, struct clientResources* rsrc, struct fileInfo* task){
    char codeBuffer[FILE_CODE_LEN] = "GET_FILE_LIST";
    int len;
    if(rsrc == NULL || task == NULL)
        return -1;
    
    // make a GET_FILE_LIST request 
    if(write(socket, codeBuffer, FILE_CODE_LEN) != FILE_CODE_LEN)
        return -2;
    
    //  receive (path_length, actual_path, version) file pairs until length 0 is encountered
    while(1){

        // get path length, 0 length indicates the end of the file list, max path is fixed so longer paths are omitted 
        if(read(socket, &len, sizeof(int)) != sizeof(int))
            return -3;
        if(len == 0)
            break;
        else if(len > PATH_SIZE-1)
            continue;

        // get actual path, ensure it is null-terminated, then get file version
        if((read(socket, &(task->path), len) != len) || (read(socket, &(task->version), sizeof(int)) != sizeof(int)))
            return -4;    
        task->path[len-1] = '\0';

        // add a GET_FILE task to the circularBuffer for another worker to complete
        if(addTask(task, rsrc) < 0)
            return -5;
    }
    return 0;
}

int getFile(int socket, struct clientResources* rsrc, struct fileInfo* task){
    char codeBuffer[FILE_CODE_LEN], fileSlice[SOCKET_CAPACITY];
    int version, fileSize, sizeRead, sliceSize, localFile;
    if(rsrc == NULL || task == NULL)
        return -1;

    // ensure that the client is really a member of the client list
    if(confirmClient(&(task->owner), rsrc) != 1)    
        return -2;

    // ask for the file
    strcpy(codeBuffer, "GET_FILE");
    if(write(socket, codeBuffer, FILE_CODE_LEN) != FILE_CODE_LEN)
        return -3;

    // get current version of file, if it exists on this system, else it will be set to -1, so it is always OUT_DATED  
    version = statFile(task->path);

    // specify file path on client's file system and version on this system
    if(write(socket, &(task->path), PATH_SIZE) < 0 || write(socket, &version, sizeof(int)) != sizeof(int))
        return -4;

    // get response
    if(read(socket, codeBuffer, FILE_CODE_LEN) != FILE_CODE_LEN)
        return -5;

    // if file is already up to date, do nothing
    if(strcmp(codeBuffer, "FILE_UP_TO_DATE") == 0)
        return 0;
    
    // else get file copy
    else if(strcmp(codeBuffer, "FILE_SIZE") == 0){
        fileSize = atoi(codeBuffer);

        // create or truncate file copy of this system
        localFile = open(task->path, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMS);

        // until the whole file is copied
        for(sizeRead = 0; sizeRead < fileSize; sizeRead += sliceSize){
            
            // get file in SOCKET_CAPACITY slices
            if((sliceSize = read(socket, fileSlice, SOCKET_CAPACITY)) < 0)
                return -7;

            // put each slice in the local file
            if(write(localFile, fileSlice, sliceSize) < 0)
                return -8;
        }
        if(sizeRead != fileSize)
            return -9;

    }

    // else response code was incorrect
    else 
        return -10;
}
