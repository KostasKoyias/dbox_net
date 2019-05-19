#include "include/dbserverOperations.h"
#include "include/clientInfo.h"

// send the whole client list via a socket to the client who made the request
int sendClients(int socket, struct G_list* list){
    return 0;
}

int clientlistUpdate(uint8_t operationCode, int socket, struct G_list* list){
    struct clientInfo info;
    if(list == NULL || socket < 0)
        return -1;

    // get ip address and port of client
    if(getClientInfo(socket, &info) != 0)
        return -2;

    // if a client logged in, add client to the client list
    if(operationCode == CLIENT_INSERT)
        return listInsert(list, &info);
    // else remove client from the client list
    else if(operationCode == CLIENT_DELETE)
        return listDelete(list, &info);
    else return -2;
}