#include <stdio.h>
#include "include/define.h"
#include "include/clientInfo.h"
#include "include/list.h"
#include "include/utils.h"
#include "include/dbserverOperations.h"

uint8_t on = 1;
void handler(int);
int main(int argc, char* argv[]){
    int sock;
    uint16_t portNumber;
    char requestCode[CODE_LEN];
    struct sockaddr_in clientAddress;
    socklen_t clientlen;
    struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
    signal(SIGINT, handler);

    // ensure usage is correct
    if(argc != 3 || strcmp(argv[1], "-p") != 0 || (portNumber = atoi(argv[2]) <= 0)){
        fprintf(stderr, "Usage: %s -p port\nNote: port number should be positive\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // create socket
    if((sock = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbserver: socket creation failed!");

    // bind socket to any address
    if(bindOnPort(sock, portNumber) == -1)
        perror_exit("dbserver: assigning an address to socket failed");

    // mark socket as a passive one
    if(listen(sock, BACK_LOG) == -1)
        perror_exit("dbserver: marking socket as a listening one failed");

    // accept connections until an interrupt signal is caught
    while(on){

        // accept TCP connection
        if(accept(sock, (struct sockaddr*)&clientAddress, &clientlen) == -1)
            perror_exit("dbserver: accepting connection failed");

        // get code of request
        if(read(sock, requestCode, CODE_LEN) == -1){
            perror("dbserver: failed to get request from client");
            continue;
        }

        // ensure request string is terminated
        requestCode[CODE_LEN-1] = '\0';

        // if a client requested to log in
        if(strcmp(requestCode, "LOG_ON") == 0)
            insertClient(sock, &clientlist);
        // else if a client asks for the client list, send it right away based on the protocol 
        else if(strcmp(requestCode, "GET_CLIENTS") == 0)
            sendClients(sock, &clientlist);
        // else if a client just logged out, remove the client from our client list
        else if(strcmp(requestCode, "LOG_OF"))
            deleteClient(sock, &clientlist);
        else 
            fprintf(stderr, "dbserver: got invalid request code %s\n", requestCode);
    }

    listPrint(&clientlist);
    listFree(&clientlist);
    fprintf(stdout, "dbserver: exiting now...\n");
    return 0;
}

void handler(int sig){
    on = 0;
    fprintf(stdout, "dbserver: caught SIGINT, handle pending requests and exit immediately\n");
}