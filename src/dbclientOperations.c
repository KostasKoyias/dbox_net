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
int getClients(int socket, struct G_list *list){
    char requestCode[CODE_LEN] = "GET_CLIENTS";
    struct clientInfo info;
    int i, len;


    // ask for the list
    if(write(socket, requestCode, CODE_LEN) != CODE_LEN)
        return -1;

    // get size of list
    if(read(socket, &len, sizeof(int)) != sizeof(int))
        return -2;

    // for each list member copy information to a local list
    for(i = 0; i < len; i++){
        if(read(socket, &(info.ipAddress), sizeof(uint32_t)) == -1 || read(socket, &(info.portNumber), sizeof(uint16_t)) == -1)
            return -3-i;
        listInsert(list, &info);
    }
    return 0;
}