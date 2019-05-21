#include <stdio.h>
#include "../include/define.h"
#include "../include/clientInfo.h"
#include "../include/list.h"
#include "../include/utils.h"
#include "../include/dbserverOperations.h"

struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
void handler(int);
int main(int argc, char* argv[]){
    int listeningSocket, responseSocket;
    uint16_t portNumber;
    char requestCode[CODE_LEN];
    struct sockaddr_in clientAddress, peer;
    socklen_t clientlen = sizeof(struct sockaddr_in), peerlen = sizeof(struct sockaddr_in);

    // declare handler in case of an interrupt signal received
    signal(SIGINT, handler);

    // ensure usage is correct
    if(argc != 3 || strcmp(argv[1], "-p") != 0 || (portNumber = (uint16_t)atoi(argv[2])) <= 0)
        error_exit("Usage: %s -p port\nNote: port number should be positive\n", argv[0]);
    
    printf("%d\n", portNumber);

    // create a listening socket, bind it to an address and mark it as a passive one 
    if((listeningSocket = getListeningSocket(portNumber)) < 0)
        perror_exit("dbserver: failed to get a listening socket");

    // accept connections until an interrupt signal is caught
    while(1){
        fprintf(stdout, "\ndbserver: handling requests on port %hu(h)/%hu(n)\n", portNumber, htons(portNumber));

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
        fprintf(stdout, "dbserver: received request '%s' from %u %hu\n", requestCode, clientAddress.sin_addr.s_addr, clientAddress.sin_port);

        // try and satisfy request, if valid and possible
        if(handleRequest(requestCode, responseSocket, &clientlist) < 0)
                fprintf(stderr, "dbserver: failed to completely satisfy \"%s\" request\n", requestCode);
    
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