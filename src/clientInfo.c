#include "include/clientInfo.h"

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