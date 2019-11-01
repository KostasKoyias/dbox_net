/*************************************************************************************************************\
 * The following are some short utility functions on linux networking, grouping together some common commands *
\*************************************************************************************************************/
#include "utils.h"

// create a socket, declaring that the address to be assigned to it, is reusable, bind a name to it if specified
int getReuseAddressSocket(){
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
int connectTo(struct sockaddr_in* server){
    int newSocket;
    if(server == NULL)
        return -1;

    // create a socket to connect right away, server does not need the address of this client
    if((newSocket = getReuseAddressSocket()) == -1)
        return -1;

    // connect to server
    if(connect(newSocket, (struct sockaddr*)server, sizeof(*server)) == -1)
        return -3;
    return newSocket;
}

// create a listening socket, bind it to an address and mark it as a passive one 
int getListeningSocket(uint32_t ip, uint16_t port){
    int listeningSocket;
    struct sockaddr_in address = {.sin_addr.s_addr = ip, .sin_port = port, .sin_family = AF_INET};    

    // create socket and bind socket to a re-usable address accepting connections from all network interfaces
    if((listeningSocket = getReuseAddressSocket()) == -1)
        return -1;

    // bind socket to an address
    if(ip == 0)
        bindOnPort(listeningSocket, port);
    else if(bind(listeningSocket, (struct sockaddr*)&address, sizeof(address)) < 0)
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

// convert from hostname or number-and-dots notation into binary form in network byte order
int getIpOf(char* hostname, struct in_addr* address){
    struct hostent *hostInfo;

    if(hostname == NULL || address == NULL)
        return -1;
    if((hostInfo = gethostbyname(hostname)) == NULL){
        if(inet_aton(hostname, address) < 0)
            return -2;
    }else
        *address = (*(struct in_addr*)(hostInfo->h_addr_list[0]));
    return 0;
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

// convert sockaddr_in to clientInfo
int addrToInfo(struct sockaddr_in* addr, struct clientInfo* info){
    if(addr == NULL || info  == NULL)
        return -1;
    info->ipAddress = (uint32_t)addr->sin_addr.s_addr;
    info->portNumber = (uint16_t)addr->sin_port;
    return 0;
}

// convert clientInfo to sockaddr_in
int infoToAddr(struct clientInfo* info, struct sockaddr_in* addr){
    if(addr == NULL || info  == NULL)
        return -1;
    addr->sin_addr.s_addr = (in_addr_t)info->ipAddress;
    addr->sin_port =  (in_port_t)info->portNumber;
    return 0;
}