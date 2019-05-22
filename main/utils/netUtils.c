/*************************************************************************************************************\
 * The following are some short utility functions on linux networking, grouping together some common commands *
\*************************************************************************************************************/
#include "../include/utils.h"

// create a socket, declaring that the address to be assigned to it, is reusable
int getReuseAddrSocket(){
    int newSocket, option = 1;

    // create new socket
    if((newSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        return -1;

    // make address re-usable
    if(setsockopt(newSocket, 1, (SO_REUSEADDR | SO_REUSEPORT) , &option, sizeof(int)) < 0)
        return -2;
    return newSocket;
}


// given a client-server pair of addresses establish a connection between them
// letting the server know who the client is by assigning the address of the client to his socket
int establishConnection(struct sockaddr_in* client, struct sockaddr_in* server){
    int newSocket, option = 1;

    // create new socket
    if((newSocket = getReuseAddrSocket()) < 0)
        return -1;

    // associate this socket with the client's address
    if(bind(newSocket, (struct sockaddr*)client, sizeof(*client)) < 0)
        return -2;    

    // connect to server
    if(connect(newSocket, (struct sockaddr*)server, sizeof(*server)) == -1)
        return -3;
    return newSocket;
}

// create a listening socket, bind it to an address and mark it as a passive one 
int getListeningSocket(int portNumber){
    int listeningSocket;

    // create socket
    if((listeningSocket = getReuseAddrSocket()) == -1)
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

// return address of this machine into binary form in network byte order
int getMyIp(struct in_addr* address){
    char hostname[HOST_SIZE];
    struct hostent *hostInfo;
    if(gethostname(hostname, HOST_SIZE) == -1)
        return -1;

    // get IP address of this machine
    if((hostInfo = gethostbyname(hostname)) == NULL)
        return -2;
    *address =  (*(struct in_addr*)hostInfo->h_addr_list[0]);
    return 0;
}

// check whether a file exists on this file system and return the lastest version id
int statFile(char* path){
    struct stat statBuffer;

    // get status of file, return FILE_NOT_FOUND in case of failure
    if(stat(path, &statBuffer) == -1)
        return -1;
    else 
        return (int)statBuffer.st_mtime;
}