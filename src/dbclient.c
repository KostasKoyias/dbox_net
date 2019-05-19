#include <stdio.h>
#include "include/define.h"
#include "include/clientInfo.h"
#include "include/list.h"
#include "include/utils.h"
#define ARGC 13
#define HOST_SIZE 256

void usage_error(const char*);
struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
void handler(int);

int main(int argc, char* argv[]){
    int i, dirName, workerThreads = 0, bufferSize = 0, sock;
    struct sockaddr_in server = {.sin_addr.s_addr = 0, .sin_family = AF_INET, .sin_port = 0}, client = {.sin_family = AF_INET};
    char requestCode[CODE_LEN], hostName[256];
    struct hostent* clientHost;

    signal(SIGINT, handler); // in case of a ^C signal

    // handle command line arguments, ensure they are all in the appropriate range
    //if(argc != ARGC)
    //    usage_error(argv[0]);
    for(i = 1; i < argc; i+=2){
        if(strcmp(argv[i], "-d") == 0)
            dirName = i + 1;
        else if(strcmp(argv[i], "-p") == 0)
            client.sin_port = htons(atoi(argv[i+1]));
        else if(strcmp(argv[i], "-w") == 0)
            workerThreads = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-b") == 0)
            bufferSize = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-sp") == 0)
            server.sin_port = htons((uint16_t)atoi(argv[i+1]));
        else if(strcmp(argv[i], "-sip") == 0)
            //memcpy(&server.sin_addr, argv[i+1], sizeof(argv[i+1]));
            server.sin_addr.s_addr = htonl((uint32_t)atoi(argv[i+1]));        
        else
            usage_error(argv[0]);
    }
    //if(server.sin_port <= 0 || server.sin_addr.s_addr <= 0 || client.sin_port <= 0 || workerThreads <= 0 || bufferSize <= 0)
      //  error_exit("dbclient: integer arguments should all be positive\n");

    // create socket
    if((sock = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbclient: socket creation failed!");
    
    // connect to server
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) == -1)
        perror_exit("dbclient: connect to server");
    
    // inform server for your arrival issuing a LOG_ON request
    strcpy(requestCode, "LOG_ON");
    write(sock, requestCode, CODE_LEN);

    // get hostname of this machine
    if(gethostname(hostName, HOST_SIZE) == -1)
        perror_exit("dbclient: getting hostname of client");
    
    // get IP address of this machine
    if((clientHost = gethostbyname(hostName)) == NULL)
        perror_exit("dbclient: getting IP address of client");

    // convert IP address to binary
    inet_aton(clientHost->h_addr_list[0], &client.sin_addr);

    // send (IP address, port) pair to server
    write(sock, &client.sin_addr.s_addr, sizeof(uint32_t));
    write(sock, &client.sin_port, sizeof(uint16_t));
    return 0; 
}

// print a usage message and exit with code 1(FAILURE)
void usage_error(const char *path){
    fprintf(stderr, "Usage: %s –d dirName –p portNum –w workerThreads –b bufferSize –sp serverPort –sip serverIP\n", path);
    exit(EXIT_FAILURE);
}

// in case of an interrupt signal
void handler(int sig){
}


