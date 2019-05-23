#include "../include/dbclient.h"

// when a connection is established and a request is made from another client, main thread needs to handle it
int handleClientRequest(char* path, char* requestCode, int socket, struct clientResources* rsrc){

    if(path == NULL || requestCode == NULL || rsrc == NULL)
        return -1;

    // if a client requested to log in all file paths under input directory, send them through the socket
    if(strcmp(requestCode, "GET_FILE_LIST") == 0)
        return handleGetFileList(socket, path);

    // else if a client asked for a specific file, send it right away based on the protocol 
    else if(strcmp(requestCode, "GET_FILE") == 0)
        return handleGetFile(socket, path);
    
    // else request code is invalid
    else
        return -1;
}

// respond to a "GET_FILE_LIST" request by sending all file paths under input directory
int handleGetFileList(int socket, char* dirPath){
    struct dirent* filePointer;
    struct stat statBuffer;
    static int flag = -1, inputlen;    // retains information through out recursive calls
    int pathSize, isRoot = 0;
    char *fullPath, *relativePath;
    DIR* dirPointer;
    if(dirPath == NULL)
        return -1;
    
    // first call to this recursive function is responsible for letting peer know that transaction was completed
    if(flag == -1){
        isRoot = flag = 1;
        inputlen = strlen(dirPath);
    }

    // open input directory
    if((dirPointer = opendir(dirPath)) == NULL)
        return -2;
    
    while((filePointer = readdir(dirPointer)) != NULL){

        // omit current, parent, hidden files/directories
        if(filePointer->d_name[0] == '.')
            continue;

        // get status of file under full path
        fullPath = malloc(strlen(dirPath) + strlen(filePointer->d_name) + 2);
        sprintf(fullPath, "%s/%s", dirPath, filePointer->d_name);
        if(stat(fullPath, &statBuffer) == -1){
            closedir(dirPointer);
            free(fullPath);
            return -3;
        }
        
        // if it is a directory, repeat recursively
        if(S_ISDIR(statBuffer.st_mode))
            handleGetFileList(socket, fullPath);

        // else if it is a regular file send it throught the socket
        else{

            // first let peer know about the path size, that many bytes will later on be read, omit too long paths, circularBuffer.path is fixed
            relativePath = fullPath + inputlen + 1; // send a path relative to your input directory
            pathSize = strlen(relativePath);
            printf("SEND %s\n", relativePath);//@@@@@@@@@@@

            if(pathSize >= PATH_SIZE-1 || write(socket, &pathSize, sizeof(int)) != sizeof(int) || (write(socket, relativePath, pathSize) != pathSize)){
                closedir(dirPointer);
                free(fullPath);
                return -4;
            }

            // then send current file version
            if(write(socket, (int*)(&(statBuffer.st_mtime)), sizeof(int)) != sizeof(int)){
                closedir(dirPointer);
                free(fullPath);
                return -5;
            }
        }
        free(fullPath);
    }

    // inform peer that transaction is completed by sending path length equal to 0
    if(isRoot){
        flag = 0;
        if(write(socket, &flag, sizeof(int)) != sizeof(int)){
            closedir(dirPointer);
            return -6;
        }

    }
    closedir(dirPointer);
    return 0;
}

// respond to a "GET_FILE" request by getting the exact path and sending all bytes of the file under that path
int handleGetFile(int socket, char* directoryPath){
    char fullPath[PATH_SIZE], relativePath[PATH_SIZE], responseCode[FILE_CODE_LEN], buffer[SOCKET_CAPACITY];
    struct stat statBuffer;
    int version, fd, bytes, pathSize;
    if(directoryPath == NULL)
        return -1;
    
    // get relative file path from peer
    if((read(socket, &pathSize, sizeof(pathSize)) != sizeof(pathSize)) || (read(socket, relativePath, pathSize) != pathSize))
        return -2;

    // append to directory path to get full path 
    sprintf(fullPath, "%s/%s", directoryPath, relativePath);

    // get version of file, return FILE_NOT_FOUND if it does not exists
    if(stat(fullPath, &statBuffer) == -1){
        printf("%s not found\n", fullPath);
        strcpy(responseCode, "FILE_NOT_FOUND");
        perror("error:");
        if(write(socket, responseCode, FILE_CODE_LEN) != FILE_CODE_LEN)
            return -3;
        return 0;
    }

    // get version number of the file copy the peer owns
    if(read(socket, &version, sizeof(int)) != sizeof(int))
        return -4;

    // if peer owns an out dated version, send the file
    if((int)statBuffer.st_mtime > version){
        
        // let client know that you are about to send the new version
        strcpy(responseCode, "FILE_SIZE");
        if(write(socket, responseCode, FILE_CODE_LEN) != FILE_CODE_LEN)
            return -5;

        // send file size
        if(write(socket, &(statBuffer.st_size), sizeof(off_t)) != sizeof(off_t))
            return -6;

        // send file content by first opening file
        if((fd = open(fullPath, FILE_PERMS)) < 0)
            return -7;

        // then put SOCKET_CAPACITY bytes at a time in the socket 
        while((bytes = read(fd, buffer, SOCKET_CAPACITY)) == SOCKET_CAPACITY){
            if(write(socket, buffer, SOCKET_CAPACITY) != SOCKET_CAPACITY)
                return -8;
        }

        if(bytes == -1)
            return -9;

        // send remaining bytes
        if(write(socket, buffer, bytes) != bytes)
            return -10;
        return 1;
    }
    // else send an appropriate message
    else{
        strcpy(responseCode, "FILE_UP_TO_DATE");
        if(write(socket, responseCode, FILE_CODE_LEN) != FILE_CODE_LEN)
            return -11;
        return 2;
    }
}

// when server informs the client about a USER_ON/OFF act appropriately
int handleServerMessage(char* requestCode, int socket, struct clientResources* rsrc){

    if(requestCode == NULL || rsrc == NULL)
        return -1;

    // if this is a message from the server specifying a new user, make sure to get all new files
    if(strcmp(requestCode, "USER_ON") == 0)
        return handleUser(USER_ON, socket, rsrc);

    // else if this is a message from the server about a user exiting the system, remove user from user list
    else if(strcmp(requestCode, "USER_OFF") == 0)
        return handleUser(USER_OFF, socket, rsrc); 

    // else request code is invalid
    else
        return -1;
}

// handle a request from server about a new or exiting user
int handleUser(int code, int socket, struct clientResources* rsrc){
    struct fileInfo fileInfo = {.path = "\0", .version = -1}; //version -1 indicates that this is a GET_FILE_LIST task
    if(rsrc == NULL)
        return -1;

    // get (IP, port) pair of the client
    if(getClientInfo(socket, &(fileInfo.owner)) < 0)
        return -2;
    
    if(code == USER_ON)
        return addClient(&fileInfo, rsrc);
    else if(code == USER_OFF)
        return removeClient(&(fileInfo.owner), rsrc);
    else 
        return -3;
}
