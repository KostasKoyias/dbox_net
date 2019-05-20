#include <stdio.h>
#include "include/define.h"
#include "include/clientInfo.h"
#include "include/list.h"
#include "include/utils.h"
#include "include/dbserverOperations.h"

struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
void handler(int);
int main(int argc, char* argv[]){
    int listeningSocket, responseSocket;
    uint16_t portNumber;
    char requestCode[CODE_LEN];
    struct sockaddr_in clientAddress;
    socklen_t clientlen = 0;

    // declare handler in case of an interrupt signal received
    signal(SIGINT, handler);

    // ensure usage is correct
    if(argc != 3 || strcmp(argv[1], "-p") != 0 || (portNumber = (uint16_t)atoi(argv[2])) <= 0)
        error_exit("Usage: %s -p port\nNote: port number should be positive\n", argv[0]);
    
    printf("%d\n", portNumber);

    // create socket
    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbserver: socket creation failed!");

    // bind socket to an address
    if(bindOnPort(listeningSocket, portNumber) == -1)
        perror_exit("dbserver: assigning an address to socket failed");

    // mark socket as a passive one
    if(listen(listeningSocket, BACK_LOG) == -1)
        perror_exit("dbserver: marking socket as a listening one failed");

    // accept connections until an interrupt signal is caught
    while(1){
        fprintf(stdout, "dbserver: handling requests on port %hu(h)/%hu(n)\n", portNumber, htons(portNumber));

        // accept TCP connection
        if((responseSocket = accept(listeningSocket, (struct sockaddr*)&clientAddress, &clientlen)) == -1)
            perror_exit("dbserver: accepting connection failed");

        // get code of request
        if(read(responseSocket, requestCode, CODE_LEN) != CODE_LEN){
            perror("dbserver: failed to get request from client");
            continue;
        }

        // ensure request string is terminated
        requestCode[CODE_LEN-1] = '\0';
        fprintf(stdout, "request %s from %u %hu\n", requestCode, clientAddress.sin_addr.s_addr, ntohs(clientAddress.sin_port));
    
        // if a client requested to log in, add client to the client list
        if(strcmp(requestCode, "LOG_ON") == 0 && clientsUpdate(CLIENT_INSERT, responseSocket, &clientlist) < 0)
            fprintf(stderr, "dbserver: failed to add new client\n");

        // else if a client just asked for the client list, send it right away based on the protocol 
        else if(strcmp(requestCode, "GET_CLIENTS") == 0 && sendClients(responseSocket, &clientlist) < 0)
            fprintf(stderr, "dbserver: failed to send client list to a new client\n");

        // else if a client just logged out, remove the client from our client list
        else if(strcmp(requestCode, "LOG_OFF") == 0 && clientsUpdate(CLIENT_DELETE, responseSocket, &clientlist) < 0)
            fprintf(stderr, "dbserver: failed to satisfy LOG_OFF request\n");
    
        // close response socket, not to run out of file descriptors
        close(responseSocket);
        listPrint(&clientlist);

    }
    return 0;
}

void handler(int sig){
    listFree(&clientlist);
    fprintf(stdout, "dbserver: exiting now...\n");
    exit(EXIT_SUCCESS);
}