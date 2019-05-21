/***********************************************************************************************\
 * The following are some short utility functions, making code re-use simpler                   *
 * most of them handle common error cases and code that is widely used through out this project *
\***********************************************************************************************/
#include "../include/utils.h"

// print error indicated by errno and exit with code 1(FAILURE)
void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

// print a message in stderr and exit with code 1(FAILURE)
void error_exit(const char* format,...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

// print a message in stderr and return $ret_val to the caller
int error_return(int ret_val, const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    return ret_val;
}

// lock a file and print a message in it
int exclusive_print(int fd, const char* format, ...){

    // get arguments
    va_list args;
    va_start(args, format);

    // lock file,print message and flush to the disc
    flock(fd, LOCK_EX);
    vdprintf(fd, format, args);
    fsync(fd);
    flock(fd, LOCK_UN);
    va_end(args);
    return 0;
}

// create a listening socket, bind it to an address and mark it as a passive one 
int getListeningSocket(int portNumber){
    int listeningSocket;

    // create socket
    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        return -1;

    // bind socket to an address
    if(bindOnPort(listeningSocket, portNumber) == -1)
        return -2;

    // mark socket as a passive one
    if(listen(listeningSocket, BACK_LOG) == -1)
        return -3;
    return listeningSocket;
}


// bind a socket to an address, accepting connections from anywhere
int bindOnPort(int fd, uint16_t portNumber){
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portNumber);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return bind(fd, (struct sockaddr*)&addr, sizeof(addr));
}

int getRequest(char* name, char** requestCode, int listeningSocket, int portNumber, struct sockaddr_in* client, socklen_t* clientlen){
    int socket;
    if(requestCode == NULL || name == NULL || client == NULL || clientlen == NULL)
        return -1;

    if((*requestCode = malloc(sizeof(char) * CODE_LEN)) < 0)
        return -2;
    fprintf(stdout, "%s: handling requests on port %hu(h)/%hu(n)\n", name, portNumber, htons(portNumber));

    // accept TCP connection
    if((socket = accept(listeningSocket, (struct sockaddr*)client, clientlen)) == -1)
        return -3;

    /**********************************************/
    printf("Accepted connection from: %d\t%d\n", htonl(client->sin_addr.s_addr), htons(client->sin_port));
    /*********************************************/

    // get code of request
    if(read(socket, *requestCode, CODE_LEN) != CODE_LEN)
        return -4;
    return socket;

}