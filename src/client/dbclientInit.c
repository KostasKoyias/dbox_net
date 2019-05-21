#include "../include/dbclientOperations.h"
#include "../include/clientInfo.h"

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
    int i, len;

    // ask for the list
    if(write(socket, requestCode, CODE_LEN) != CODE_LEN)
        return -1;

    // get size of list
    if(read(socket, &len, sizeof(int)) != sizeof(int))
        return -2;

    // for each list member
    for(i = 0; i < len; i++){
        if(read(socket, &(client.ipAddress), sizeof(uint32_t)) == -1 || read(socket, &(client.portNumber), sizeof(uint16_t)) == -1)
            return -4-2*i;

        // omit own registration
        if((client.ipAddress == myAddress->sin_addr.s_addr) && (client.portNumber == myAddress->sin_port))
            continue;

        // add new client to both the buffer and the client list
        if(addClient(&client, rsrc) < 0)
            return -5-2*i;
    }
    return 0;
}