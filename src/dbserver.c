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
        if(read(responseSocket, requestCode, CODE_LEN) == -1){
            perror("dbserver: failed to get request from client");
            continue;
        }

        // ensure request string is terminated
        requestCode[CODE_LEN-1] = '\0';

        // if a client requested to log in
        if(strcmp(requestCode, "LOG_ON") == 0)
            clientlistUpdate(CLIENT_INSERT, responseSocket, &clientlist);
        // else if a client asks for the client list, send it right away based on the protocol 
        else if(strcmp(requestCode, "GET_CLIENTS") == 0)
            sendClients(listeningSocket, &clientlist);
        // else if a client just logged out, remove the client from our client list
        else if(strcmp(requestCode, "LOG_OF"))
            clientlistUpdate(CLIENT_DELETE, responseSocket, &clientlist);
        else 
            fprintf(stderr, "dbserver: got invalid request code %s\n", requestCode);
        listPrint(&clientlist);

    }
    return 0;
}

void handler(int sig){
    listFree(&clientlist);
    fprintf(stdout, "dbserver: exiting now...\n");
    exit(EXIT_SUCCESS);
}