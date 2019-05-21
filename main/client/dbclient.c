#include <stdio.h>
#include "../include/define.h"
#include "../include/clientInfo.h"
#include "../include/circularBuffer.h"
#include "../include/list.h"
#include "../include/utils.h"
#include "../include/dbclientOperations.h"
#define ARGC 13
#define HOST_SIZE 256

void usage_error(const char*);
void handler(int);

int main(int argc, char* argv[]){
    int i, dirName, workerThreads = 0, bufferSize = 0, generalSocket, listeningSocket;
    struct sockaddr_in server = {.sin_addr.s_addr = 0, .sin_family = AF_INET, .sin_port = 0}, client = {.sin_family = AF_INET}, otherClient = {.sin_family = AF_INET};
    char hostName[HOST_SIZE], requestCode[FILE_CODE_LEN];
    struct clientResources rsrc = {.list = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL}};
    struct hostent* clientAddress;
    struct clientInfo peerInfo;
    socklen_t otherClientlen = sizeof(struct sockaddr);
    struct stat statBuffer;
    pthread_t* threadVector;

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
            client.sin_port = htons((uint16_t)atoi(argv[i+1]));

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
    /*if(server.sin_port <= 0 || server.sin_addr.s_addr <= 0 || client.sin_port <= 0 || workerThreads <= 0 || bufferSize <= 0)
        error_exit("dbclient: Error, all arguments, other than 'dirname' should be positive integers\n");

    if((stat(argv[dirName], &statBuffer) == -1) || (!S_ISDIR(statBuffer.st_mode)))
        error_exit("dbclient: Error, input path \"%s\" does not refer to an actual directory under this file system\n", argv[dirName]);*/

    // get hostname of this machine
    if(gethostname(hostName, HOST_SIZE) == -1)
        perror_exit("dbclient: getting hostname of client");
    
    // get IP address of this machine
    if((clientAddress = gethostbyname(hostName)) == NULL)
        perror_exit("dbclient: getting IP address of client");
    client.sin_addr = (*(struct in_addr*)clientAddress->h_addr_list[0]);

    // create socket
    if((generalSocket = socket(AF_INET , SOCK_STREAM , 0)) == -1)
        perror_exit("dbclient: server socket creation failed!");




    //***********************************************************************************************************************************
    bind(generalSocket, (struct sockaddr*)&clientAddress, sizeof(client));
    //***********************************************************************************************************************************






    // connect to server
    if(connect(generalSocket, (struct sockaddr*)&server, sizeof(server)) == -1)
        perror_exit("dbclient: connect to server");
    
    // inform server for your arrival issuing a LOG_ON request
    if(informServer(LOG_ON, generalSocket, &client) < 0)
        perror_exit("dbclient: failed inform server on arrival");

    // initialize circularBuffer, mutexes and conditions
    rsrcInit(&rsrc, bufferSize);

    // kick-start worker threads
    if((threadVector = malloc(sizeof(pthread_t) * workerThreads)) == NULL)
        perror_exit("dbclient: failed to allocate space for a thread id vector");
    for(i = 0; i < workerThreads; i++){
        if(pthread_create(threadVector+i, NULL, dbclientWorker, &rsrc))
            perror_exit("dbclient: failed to create worker thread");
    }






    // *****************************************************************Re Connect to server****************************************************************
    close(generalSocket);
    generalSocket = socket(AF_INET , SOCK_STREAM , 0);
    bind(generalSocket, (struct sockaddr*)&clientAddress, sizeof(client));    
    connect(generalSocket, (struct sockaddr*)&server, sizeof(server));
    // *****************************************************************************************************************************************************








    // ask for the client list
    if(getClients(generalSocket, &client, &rsrc) < 0)
        perror_exit("dbclient: failed to get dbox client list from server");
    bufferPrint(&(rsrc.buffer));
    listPrint(&(rsrc.list));

    // create a listening socket, bind it to an address and mark it as a passive one 
    if((listeningSocket = getListeningSocket(client.sin_port)) < 0)
        perror_exit("dbclient: failed to get a listening socket");

    // accept connections until an interrupt signal is caught
    /*while(1){
        fprintf(stdout, "dbserver: handling requests on port %hu(h)/%hu(n)\n", client.sin_port, htons(client.sin_port));

        // accept TCP connection
        if((generalSocket = accept(listeningSocket, (struct sockaddr*)&otherClient, &otherClientlen)) == -1)
            perror_exit("dbserver: accepting connection failed");

        // if peer is neither the server nor a client from the list, close connection immediately
        peerInfo.ipAddress = otherClient.sin_addr.s_addr; peerInfo.portNumber = otherClient.sin_port;
        if((server.sin_addr.s_addr != otherClient.sin_addr.s_addr || server.sin_port != otherClient.sin_port) && confirmClient(&peerInfo, &rsrc) != 1){
            close(generalSocket);
            continue;
        }

        // **************************************    DEPRECATED   *******************************************************
        printf("Accepted connection from: %d\t%d\n", htonl(otherClient.sin_addr.s_addr), htons(otherClient.sin_port));
        // **************************************************************************************************************

        // get code of request
        if(read(generalSocket, requestCode, FILE_CODE_LEN) != FILE_CODE_LEN){
            perror("dbserver: failed to get request from client");
            continue;
        }

        // ensure request string is terminated
        requestCode[FILE_CODE_LEN-1] = '\0';
        fprintf(stdout, "request %s from %u %hu\n", requestCode, otherClient.sin_addr.s_addr, otherClient.sin_port);

        handleRequest(argv[dirName], requestCode, generalSocket, &rsrc);
    
        // close response socket, not to run out of file descriptors
        close(generalSocket);
    }*/
    






    // let server know that you are about to exit dbox system issuing a LOG_OFF request
    getchar();
    // *****************************************************************Re Connect to server****************************************************************
    close(generalSocket);
    generalSocket = socket(AF_INET , SOCK_STREAM , 0);
    bind(generalSocket, (struct sockaddr*)&clientAddress, sizeof(client));    
    connect(generalSocket, (struct sockaddr*)&server, sizeof(server));
    // *****************************************************************************************************************************************************
 







    if(informServer(LOG_OFF, generalSocket, &client) < 0)
        perror_exit("dbclient: failed to inform server before exiting");
    
    close(listeningSocket);
    close(generalSocket);
    rsrcFree(&rsrc);
    free(threadVector);
    return 0; 
}

// print a usage message and exit with code: EXIT_FAILURE
void usage_error(const char *path){
    fprintf(stderr, "Usage: %s –d dirName –p portNum –w workerThreads –b bufferSize –sp serverPort –sip serverIP\n", path);
    exit(EXIT_FAILURE);
}

// in case of an interrupt signal
void handler(int sig){
    exit(EXIT_SUCCESS);
}


