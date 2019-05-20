#include "include/dbclientOperations.h"
#include "include/clientInfo.h"

// let server know about a new or an outgoing client 
int informServer(uint8_t code, int socket, struct sockaddr_in* clientAddress){
    char requestCode[CODE_LEN] = "\0";
    if(code == LOG_ON)
        strcpy(requestCode, "LOG_ON");
    else if(code == LOG_OFF) 
        strcpy(requestCode, "LOG_OFF");
    else
        return -1;
    if(write(socket, requestCode, CODE_LEN) != CODE_LEN)
        return -2;

    // send (IP address, port) pair to server
    if(write(socket, &(clientAddress->sin_addr.s_addr), sizeof(uint32_t)) != sizeof(uint32_t))
        return -3;
    
    if(write(socket, &(clientAddress->sin_port), sizeof(uint16_t)) != sizeof(uint16_t))
        return -4;
    return 0;
}

// get client list from server
int getClients(int socket, struct sockaddr_in* myAddress, struct clientResources* rsrc){
    char requestCode[CODE_LEN] = "GET_CLIENTS";
    struct clientInfo client;
    struct fileInfo file = {.path = "\0", .version = -1}; // version -1 indicates that this is a GET_FILE_LIST task
    int i, len;

    // ask for the list
    if(write(socket, requestCode, CODE_LEN) != CODE_LEN)
        return -1;

    // get size of list
    if(read(socket, &len, sizeof(int)) != sizeof(int))
        return -2;

    // based on the size just received by the server, initialize buffer to hold information for all files
    if(bufferInit(len, &(rsrc->buffer)) < 0)
        return -3;

    // for each list member
    for(i = 0; i < len; i++){
        if(read(socket, &(client.ipAddress), sizeof(uint32_t)) == -1 || read(socket, &(client.portNumber), sizeof(uint16_t)) == -1)
            return -4-i;

        // omit own registration
        if((client.ipAddress == myAddress->sin_addr.s_addr) && (client.portNumber == myAddress->sin_port))
            continue;

        // add file to the circular buffer, worker threads have not yet been created, so no need for mutexes here
        clientAssign(&file.owner, &client);
        bufferAdd(&file, &(rsrc->buffer));

        // add member to a local list
        listInsert(&(rsrc->list), &client);
    }
    return 0;
}

// when a connection is established and a request is made, main thread needs to handle it
int handleRequest(char* path, char* requestCode, int socket, struct clientResources* rsrc){

    if(path == NULL || requestCode == NULL || rsrc == NULL)
        return -1;

    // if a client requested to log in all file paths under input directory, send them through the socket
    if(strcmp(requestCode, "GET_FILE_LIST") == 0)
        return sendFilePaths(socket, path);

    // else if a client asked for a specific file, send it right away based on the protocol 
    else if(strcmp(requestCode, "GET_FILE") == 0)
        return sendCertainFile(socket, path);

    // else if this is a message from the server specifying a new user, make sure to get all new files
    else if(strcmp(requestCode, "USER_ON") == 0)
        return addClient(socket, rsrc);

    // else if this is a message from the server about a user exiting the system, remove user from user list
    else if(strcmp(requestCode, "USER_OFF") == 0)
        return removeClient(socket, rsrc); 

    // else request code is invalid
    else
        return -1;

}

// respond to a "GET_FILE_LIST" request by sending all file paths under input directory
int sendFilePaths(int socket, char* path){
    return 0;
}

// respond to a "GET_FILE_LIST" request by getting the exact path and sending all bytes of the file under that path
int sendCertainFile(int socket, char* path){
    return 0;
}

// handle a "USER_ON" send by server
int addClient(int socket, struct clientResources* rsrc){
    return 0;
}

// handle a "USER_ON" send by server
int removeClient(int socket, struct clientResources* rsrc){
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