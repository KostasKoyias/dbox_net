#include "../include/dbclient.h"

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
    if(write(socket, (uint32_t*)(&(clientAddress->sin_addr.s_addr)), sizeof(uint32_t)) != sizeof(uint32_t))
        return -3;
    
    if(write(socket, (uint16_t*)(&(clientAddress->sin_port)), sizeof(uint16_t)) != sizeof(uint16_t))
        return -4;
    return 0;
}

// get client list from server
int getClients(int socket, struct sockaddr_in* myAddress, struct clientResources* rsrc){
    struct fileInfo fileInfo = {.path = "\0", .version = -1}; //version -1 indicates that this is a GET_FILE_LIST task
    int i, len;

    // get size of list
    if(read(socket, &len, sizeof(int)) != sizeof(int))
        return -2;

    // if client already exists, abort
    if(len == ERROR_IP_PORT_EXISTS)
        return -3;

    // for each list member
    for(i = 0; i < len; i++){
        if(read(socket, &(fileInfo.owner.ipAddress), sizeof(uint32_t)) == -1 || read(socket, &(fileInfo.owner.portNumber), sizeof(uint16_t)) == -1)
            return -4-2*i;

        // omit own registration
        if((fileInfo.owner.ipAddress == myAddress->sin_addr.s_addr) && (fileInfo.owner.portNumber == myAddress->sin_port))
            continue;

        // add new client to both the buffer and the client list
        if(addClient(&fileInfo, rsrc) < 0)
            return -5-2*i;
    }
    return 0;
}