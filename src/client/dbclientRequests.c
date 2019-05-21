#include "../include/dbclientOperations.h"
#include "../include/clientInfo.h"

// when a connection is established and a request is made, main thread needs to handle it
int handleRequest(char* path, char* requestCode, int socket, struct clientResources* rsrc){

    if(path == NULL || requestCode == NULL || rsrc == NULL)
        return -1;

    // if a client requested to log in all file paths under input directory, send them through the socket
    if(strcmp(requestCode, "GET_FILE_LIST") == 0)
        return handleGetFileList(socket, path);

    // else if a client asked for a specific file, send it right away based on the protocol 
    else if(strcmp(requestCode, "GET_FILE") == 0)
        return handleGetFile(socket, path);

    // else if this is a message from the server specifying a new user, make sure to get all new files
    else if(strcmp(requestCode, "USER_ON") == 0)
        return handleUser(USER_ON, socket, rsrc);

    // else if this is a message from the server about a user exiting the system, remove user from user list
    else if(strcmp(requestCode, "USER_OFF") == 0)
        return handleUser(USER_OFF, socket, rsrc); 

    // else request code is invalid
    else
        return -1;

}

// respond to a "GET_FILE_LIST" request by sending all file paths under input directory
int handleGetFileList(int socket, char* path){
    return 0;
}

// respond to a "GET_FILE" request by getting the exact path and sending all bytes of the file under that path
int handleGetFile(int socket, char* directoryPath){
    char fullPath[PATH_SIZE], relativePath[PATH_SIZE], responseCode[FILE_CODE_LEN], buffer[SOCKET_CAPACITY];
    struct stat statBuffer;
    int version, fd, bytes;
    if(directoryPath == NULL)
        return -1;
    
    // get relative file path from peer
    if(read(socket, relativePath, PATH_SIZE) != PATH_SIZE)
        return -2;

    // append to directory path to get full path 
    sprintf(fullPath, "%s/%s", directoryPath, relativePath);

    // get status of file, return FILE_NOT_FOUND in case of failure
    if(stat(fullPath, &statBuffer) == -1){
        strcpy(responseCode, "FILE_NOT_FOUND");
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

        // send file version
        if(write(socket, (int*)(&(statBuffer.st_mtime)), sizeof(int)) != sizeof(int))
            return -6;  

        // send file size
        if(write(socket, &(statBuffer.st_size), sizeof(off_t)) != sizeof(off_t))
            return -7;

        // send file content by first opening file
        if((fd = open(fullPath, FILE_PERMS)) < 0)
            return -8;

        // then put SOCKET_CAPACITY bytes at a time in the socket 
        while((bytes = read(fd, buffer, SOCKET_CAPACITY)) == SOCKET_CAPACITY){
            if(write(socket, buffer, SOCKET_CAPACITY) != SOCKET_CAPACITY)
                return -9;
        }

        if(bytes == -1)
            return -10;

        // send remaining bytes
        if(write(socket, buffer, bytes) != bytes)
            return -11;
        return 1;
    }
    // else send an appropriate message
    else{
        strcpy(responseCode, "FILE_OUT_OF_DATE");
        if(write(socket, responseCode, FILE_CODE_LEN) != FILE_CODE_LEN)
            return -10;
        return 2;
    }
}

// handle a request from server about a new or exiting user
int handleUser(int code, int socket, struct clientResources* rsrc){
    struct clientInfo clientInfo;
    if(rsrc == NULL)
        return -1;

    // get (IP, port) pair of the client
    if(getClientInfo(socket, &clientInfo) < 0)
        return -2;
    
    if(code == USER_ON)
        return addClient(&clientInfo, rsrc);
    else if(code == USER_OFF)
        return removeClient(&clientInfo, rsrc);
    else 
        return -3;
}
