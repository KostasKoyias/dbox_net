#include <stdio.h>
#include "../include/utils.h"
#include "../include/dbserverOperations.h"

struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
void handler(int);

int main(int argc, char* argv[]){
    int listeningSocket, responseSocket;
    uint32_t port;
    char requestCode[CODE_LEN];
    struct sockaddr_in client;
    socklen_t clientlen = sizeof(struct sockaddr_in);

    // declare handler in case of an interrupt signal received
    signal(SIGINT, handler);
    signal(SIGPIPE, SIG_IGN);

    // ensure usage is correct
    if(argc != 3 || strcmp(argv[1], "-p") != 0 || (port = (uint16_t)atoi(argv[2])) <= 0)
        error_exit("Usage: %s -p port\nNote: port number should be positive\n", argv[0]);
    
    // create a listening socket, bind it to an address and mark it as a passive one 
    if((listeningSocket = getListeningSocket(0, port)) < 0)
        perror_exit("dbserver: failed to get a listening socket");

    // accept connections until an interrupt signal is caught
    while(1){
        fprintf(stdout, "\ndbserver: handling requests on port %hu(h)/%hu(n)\n", port, htons(port));

        // accept TCP connection
        if((responseSocket = accept(listeningSocket, (struct sockaddr*)&client, &clientlen)) == -1)
            perror_exit("dbserver: accepting connection failed");

        // get code of request
        if(read(responseSocket, requestCode, CODE_LEN) != CODE_LEN){
            perror("dbserver: failed to get request from client");
            continue;
        }

        // ensure request string is terminated
        requestCode[CODE_LEN-1] = '\0';
        fprintf(stdout, "dbserver: received '%s'\n", requestCode);

        // try and satisfy request, if valid and possible
        if(handleRequest(requestCode, responseSocket, &clientlist) < 0)
            perror("dbserver: failed to completely satisfy the request");
    
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