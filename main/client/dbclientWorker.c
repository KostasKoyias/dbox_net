#include "../include/dbclient.h"

// client's worker thread, implementing the client side of his, responsible for getting files of other clients 
void* dbclientWorker(void* arg){
    struct clientResources* rsrc;
    struct fileInfo task;
    struct sockaddr_in peer = {.sin_family = AF_INET};
    extern struct sockaddr_in address;
    int sock, rv;
    extern uint8_t powerOn;

    if(arg == NULL)
        return NULL;
    rsrc = (struct clientResources*)arg;
    fprintf(stdout, "thread %ld: start\n", pthread_self());

    // while client is logged in
    while(powerOn){

        // block until buffer is non-empty, then remove a task from circularBuffer
        pthread_mutex_lock(&(rsrc->bufferMutex));
        fprintf(stdout, "thread %ld: block, isEmpty? %d\n", pthread_self(), bufferIsEmpty(&(rsrc->buffer)));
        while(bufferIsEmpty(&(rsrc->buffer))){

            // atomically unlock buffer and self-block until condition holds, then lock again as soon as possible
            pthread_cond_wait(&(rsrc->emptyBuffer), &(rsrc->bufferMutex)); 
            
            // check whether this thread was signaled in order to log out
            if(!powerOn){
                pthread_mutex_unlock(&(rsrc->bufferMutex));
                goto powerOff;
            }
        }
        bufferRemove(&task, &(rsrc->buffer));
        fprintf(stdout, "thread %ld: pop task\n", pthread_self());
        pthread_cond_signal(&(rsrc->fullBuffer));   // signal a pending thread
        pthread_mutex_unlock(&(rsrc->bufferMutex));

        // ensure that the client is really a member of the client list
        if(confirmClient(&(task.owner), rsrc) != 1)    
            continue;

        // create socket, bind it to an address and establish a connection with peer
        peer.sin_addr.s_addr = task.owner.ipAddress;
        peer.sin_port = task.owner.portNumber;
        if((sock = establishConnection(&address, &peer)) < 0){
            perror("dbclient: working thread failed to establish connection with peer");
            continue;
        }

        // if task is about syncing with a new user, i.e GET_FILE_LIST then version is set to -1, else it is about getting a certain file version from another client
        if(task.version == -1)
            rv = getFileList(sock, rsrc, &task);
        else
            rv = getFile(sock, rsrc, &task);
        if(rv < 0)
            perror("dbclient: working thread failed to complete task");
        close(sock);
    }

powerOff:
    fprintf(stdout, "thread %ld: got power off!\n", pthread_self());
    pthread_exit(NULL);
}

// get a list of all files from a certain client, put each of them in the buffer for some thread to ask for their content from that other client 
int getFileList(int socket, struct clientResources* rsrc, struct fileInfo* task){
    char codeBuffer[FILE_CODE_LEN] = "GET_FILE_LIST";
    int len;
    if(rsrc == NULL || task == NULL)
        return -1;
    
    printf("GET_FILE_LIST");//@@@@@@@@@@@
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
        task->path[len] = '\0';

        // add a GET_FILE task to the circularBuffer for another worker to complete
        if(addTask(task, rsrc) < 0)
            return -5;
    }
    return 0;
}


// ask another client for a certain file
int getFile(int socket, struct clientResources* rsrc, struct fileInfo* task){
    char codeBuffer[FILE_CODE_LEN], fileSlice[SOCKET_CAPACITY], *localPath;
    int version, fileSize, sizeRead, sliceSize, localFile, rv = 0, result, pathSize, index;
    extern char* mirror;
    struct hostent* peer;
    char temp;
    if(rsrc == NULL || task == NULL)
        return -1;

    // get local path to the directory made for the other client under mirror which is of the form "mirrorPath/nameOfClient/pathClientSendYou"
    if((peer = gethostbyaddr(&(task->owner.ipAddress), sizeof(task->owner.ipAddress), AF_INET)) == NULL)
        return -2;
    if((localPath = malloc(strlen(mirror) + strlen(peer->h_name) + strlen(task->path) + 3)) < 0)
        return -3;

    // make parent directories as needed
    sprintf(localPath, "%s/%s/%s", mirror, peer->h_name, task->path);
    if((result = makeParents(localPath, DIR_PERMS)) < 0 && result != EEXIST){
        free(localPath); return -4;}

    // get current version of file, if it exists on this system, else it will be set to -1, so then, it is always OUT_DATED  
    version = statFile(localPath);

    // ask for the file
    strcpy(codeBuffer, "GET_FILE");
    if(write(socket, codeBuffer, FILE_CODE_LEN) != FILE_CODE_LEN)
        return -5;

    // specify file path on client's file system and version on this system
    pathSize = strlen(task->path);
    if(write(socket, &pathSize, sizeof(pathSize)) != sizeof(pathSize)){
        free(localPath); return -6;}

    if(write(socket, &(task->path), pathSize) != pathSize || write(socket, &version, sizeof(version)) != sizeof(version)){
        free(localPath); return -7;}

    // get response
    if(read(socket, codeBuffer, FILE_CODE_LEN) != FILE_CODE_LEN){
        free(localPath); return -8;}

    // if file is already up to date, or it was not found, do nothing
    if((strcmp(codeBuffer, "FILE_UP_TO_DATE") == 0) || (strcmp(codeBuffer, "FILE_NOT_FOUND") == 0))
        rv = 0;
            
    // else get file copy
    else if(strcmp(codeBuffer, "FILE_SIZE") == 0){

        // get size of file
        if(read(socket, &fileSize, sizeof(fileSize)) != sizeof(fileSize)){
            free(localPath); return -9;}

        // create or truncate file copy of this system, make parent directories as needed
        if((localFile = open(localPath, O_WRONLY | O_CREAT | O_TRUNC, FILE_PERMS)) < 0){
            printf("%s\n", localPath);//@@
            free(localPath);
            perror("open");//@@
            return -10;
        }

        // until the whole file is copied
        for(sizeRead = 0; sizeRead < fileSize; sizeRead += sliceSize){
            
            // get file in SOCKET_CAPACITY slices
            if((sliceSize = read(socket, fileSlice, SOCKET_CAPACITY)) < 0)
                break;

            // put each slice in the local file
            if(write(localFile, fileSlice, sliceSize) != sliceSize)
                break;
        }
        if(sizeRead != fileSize)
            rv = -11;

        close(localFile);
    }

    // else response code was incorrect
    else
        rv = -12;

    free(localPath);
    return -12;
}
