#include "include/dbserverOperations.h"
#include "include/clientInfo.h"

// insert client info read from socket into the client list
int insertClient(int socket, struct G_list* list){
    return clientlistOperation(CLIENT_INSERT, socket, list);
}

// send the whole client list via a socket to the client who made the request
int sendClients(int socket, struct G_list* list){
    return 0;
}

// delete a client from the client list
int deleteClient(int socket, struct G_list* list){
    return clientlistOperation(CLIENT_DELETE, socket, list);
}

int clientlistOperation(uint8_t operationCode, int socket, struct G_list* list){
    struct clientInfo info;
    if(list == NULL || socket < 0)
        return -1;

    // get ip address and port of client
    getClientInfo(socket, &info);

    // if a client logged in, add client to the client list
    if(operationCode == CLIENT_INSERT)
        return listInsert(list, (void*)&info);
    // else remove client from the client list
    else if(operationCode == CLIENT_DELETE)
        return listDelete(list, (void*)&info);
    else return -2;
}