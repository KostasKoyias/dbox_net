#include "../include/clientInfo.h"

/* given two clients, return the address of the 1st one, if identical, else NULL*/
void* clientCompare(void* clientA, const void*clientB){
    struct clientInfo *clientInfoA, *clientInfoB;
    if(clientA == NULL || clientB == NULL)
        return NULL;
    clientInfoA = (struct clientInfo*)clientA;
    clientInfoB = (struct clientInfo*)clientB;
    return (clientInfoA->ipAddress == clientInfoB->ipAddress && clientInfoA->portNumber == clientInfoB->portNumber) ? clientA : NULL;
}

/* assign to/update a clientInfo node */
int clientAssign(void *clientA, const void *clientB){
    struct clientInfo *clientInfoA, *clientInfoB;

    if(clientA == NULL || clientB == NULL)
        return -1;
    clientInfoA = (struct clientInfo*)clientA;
    clientInfoB = (struct clientInfo*)clientB;
    clientInfoA->ipAddress = clientInfoB->ipAddress;
    clientInfoA->portNumber = clientInfoB->portNumber;
    return 0;
}

/* given a client's info display an (ip, port) pair in stdout */
int clientPrint(void* client){
    if(client == NULL)
        return -1;
    fprintf(stdout, "Client: (%u, %hu)\n", ((struct clientInfo*)client)->ipAddress, ((struct clientInfo*)client)->portNumber);
    return 0;
}

/* get client's info from a socket */
int getClientInfo(int socket, struct clientInfo* info){
    if(socket < 0 || info == NULL)
        return -1;

    // get ip address and port of the client connected 
    if(read(socket, &(info->ipAddress), sizeof(uint32_t)) == -1 || read(socket, &(info->portNumber), sizeof(uint16_t)) == -1){
        perror("dbserver: failed to get ip address and port for the logged on client");
        return -2;
    }  
    return 0;
}