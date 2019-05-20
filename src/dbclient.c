#include <stdio.h>
#include "include/define.h"
#include "include/clientInfo.h"
#include "include/list.h"
#include "include/utils.h"
#include "include/dbclientOperations.h"
#define ARGC 13
#define HOST_SIZE 256

void usage_error(const char*);
struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL};
void handler(int);

int main(int argc, char* argv[]){
    int i, dirName, workerThreads = 0, bufferSize = 0, serverSocket;
    struct sockaddr_in server = {.sin_addr.s_addr = 0, .sin_family = AF_INET, .sin_port = 0}, client = {.sin_family = AF_INET};
    char hostName[HOST_SIZE];
    struct hostent* clientAddress;

    signal(SIGINT, handler); // in case of a ^C signal

    // handle command line arguments, ensure they are all in the appropriate range
    //if(argc != ARGC)
    //    usage_error(argv[0]);
    for(i = 1; i < argc; i+=2){
        
        // get index of input directory path in the argument vector
        if(strcmp(argv[i], "-d") == 0)
            dirName = i + 1;
        
        // convert port number from string to network byte order
        else if(strcmp(argv[i], "-p") == 0)
            client.sin_port = htons(atoi(argv[i+1]));

        // get number of threads working for this client
        else if(strcmp(argv[i], "-w") == 0)
            workerThreads = atoi(argv[i+1]);
        
        // get size of buffer shared between threads
        else if(strcmp(argv[i], "-b") == 0)
            bufferSize = atoi(argv[i+1]);

        // convert server's port number from string to network byte order 
        else if(strcmp(argv[i], "-sp") == 0)
            server.sin_port = htons((uint16_t)atoi(argv[i+1]));

        // convert server's IP address from a string in dotted format to binary
        else if(strcmp(argv[i], "-sip") == 0){
            if(inet_aton(argv[i+1], &server.sin_addr) < 0)
                perror_exit("dbclient: server ip not in presentation format");
        }
        else
            usage_error(argv[0]);
    }
    //if(server.sin_port <= 0 || server.sin_addr.s_addr <= 0 || client.sin_port <= 0 || workerThreads <= 0 || bufferSize <= 0)
      //  error_exit("dbclient: integer arguments should all be positive\n");

    // get hostname of this machine
    if(gethostname(hostName, HOST_SIZE) == -1)
        perror_exit("dbclient: getting hostname of client");
    
    // get IP address of this machine
    if((clientAddress = gethostbyname(hostName)) == NULL)
        perror_exit("dbclient: getting IP address of client");
    client.sin_addr = (*(struct in_addr*)clientAddress->h_addr_list[0]);

    // create socket
    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbclient: socket creation failed!");
    
    // connect to server
    if(connect(serverSocket, (struct sockaddr*)&server, sizeof(server)) == -1)
        perror_exit("dbclient: connect to server");
    
    // inform server for your arrival issuing a LOG_ON request
    if(informServer(LOG_ON, serverSocket, &client) < 0)
        perror_exit("dbclient: failed inform server on arrival");

    /*****************************************************************Re Connect to server****************************************************************/
    // create socket
    close(serverSocket);
    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbclient: socket creation2 failed!");
    if(connect(serverSocket, (struct sockaddr*)&server, sizeof(server)) == -1)
      perror_exit("dbclient: connect to server2");
    /*****************************************************************************************************************************************************/

    // ask for the client list
    if(getClients(serverSocket, &clientlist) < 0)
        perror_exit("dbclient: failed to get dbox client list from server");
    listPrint(&clientlist);

    // let server know that you are about to exit dbox system issuing a LOG_OFF request
    getchar();
    /*****************************************************************Re Connect to server****************************************************************/
    // create socket
    close(serverSocket);
    if((serverSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbclient: socket creation3 failed!");
    if(connect(serverSocket, (struct sockaddr*)&server, sizeof(server)) == -1)
      perror_exit("dbclient: connect to server3");
    /*****************************************************************************************************************************************************/
    if(informServer(LOG_OFF, serverSocket, &client) < 0)
        perror_exit("dbclient: failed to inform server before exiting");
    
    close(serverSocket);
    listFree(&clientlist);
    return 0; 
}

// print a usage message and exit with code: EXIT_FAILURE
void usage_error(const char *path){
    fprintf(stderr, "Usage: %s –d dirName –p portNum –w workerThreads –b bufferSize –sp serverPort –sip serverIP\n", path);
    exit(EXIT_FAILURE);
}

// in case of an interrupt signal
void handler(int sig){
}


