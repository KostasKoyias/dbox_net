#include "dbclient.h"
#define ARGC 13

// in case of an interrupt signal, dbclient shall exit safely through the handler, therefore variables used there are declared globally
pthread_t* threadVector;
int listeningSocket, workerThreads = 0;
struct clientResources rsrc = {.list = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL}};
struct sockaddr_in address = {.sin_family = AF_INET}, server = {.sin_addr.s_addr = 0, .sin_family = AF_INET, .sin_port = 0};
uint8_t powerOn = 1;
char* mirror;

int main(int argc, char* argv[]){
    int i, dirName = 0, generalSocket = 0, bufferSize = 0;
    char requestCode[FILE_CODE_LEN];
    struct clientInfo peerInfo;
    struct stat statBuffer;

    signal(SIGINT, handler); // in case of a ^C signal

    // handle command line arguments, ensure they are all in the appropriate range
    if(argc != ARGC)
        usage_error(argv[0]);
    for(i = 1; i < argc; i+=2){
        
        // get index of input directory path in the argument vector
        if(strcmp(argv[i], "-d") == 0)
            dirName = i + 1;
        
        // convert port number from string to network byte order
        else if(strcmp(argv[i], "-p") == 0)
            address.sin_port = htons((uint16_t)atoi(argv[i+1]));

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
            if(getIpOf(argv[i+1], &server.sin_addr) < 0)
                perror_free("dbclient: server ip not in presentation format");
        }
        else
            usage_error(argv[0]);
    }
    if(server.sin_port <= 0 || address.sin_port <= 0 || workerThreads <= 0 || bufferSize <= 0 || dirName <= 0)
        error_exit("dbclient: Error, all arguments, other than 'dirname'  and 'server_address' should be positive integers\n");

    if((stat(argv[dirName], &statBuffer) == -1) || (!S_ISDIR(statBuffer.st_mode)))
        error_exit("dbclient: Error, input path \"%s\" does not refer to an actual directory under this file system\n", argv[dirName]);

    // specify path for mirror directory
    if((mirror = malloc(strlen(argv[dirName]) + strlen(MIRROR) + 2)) < 0)
        perror_exit("dbclient: \e[31;1mfailed\e[0m to allocate space in order to specify the path of mirror directory");
    setPath(mirror, argv[dirName], MIRROR);

    // create mirror directory, if it does not exist and announce presence
    if((i = mkdirTree(mirror, DIR_PERMS)) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to create mirror directory at login");
    else if(i == EEXIST)
        fprintf(stdout, "dbclient: re-logged in dbox, input directory under %s and mirror under %s\n", argv[dirName], mirror);
    else
        fprintf(stdout, "dbclient: first time at dbox, input directory under %s and mirror under %s\n", argv[dirName], mirror);

    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PREPARE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

    // initialize circularBuffer, mutexes and conditions
    rsrcInit(&rsrc, bufferSize);

    // kick-start worker threads
    if((threadVector = malloc(sizeof(pthread_t) * workerThreads)) == NULL)
        perror_free("dbclient: \e[31;1mfailed\e[0m to allocate space for a thread id vector");

    for(i = 0; i < workerThreads; i++){
        if(pthread_create(threadVector+i, NULL, dbclientWorker, &rsrc))
            perror_free("dbclient: \e[31;1mfailed\e[0m to create worker thread");
    }
    
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ LOG_ON @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    // get IP address of this machine
    if(getMyIp(&(address.sin_addr)) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to get IP address of this client");

    // create a listening socket, bind it to an address and mark it as a passive one 
    if((listeningSocket = getListeningSocket(address.sin_addr.s_addr, address.sin_port)) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to get a listening socket");

    // connect to server to issue a log_on request
    if((generalSocket = connectTo(&server)) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to establish connection at LOG_ON stage");

    // inform server for your arrival issuing a LOG_ON request
    if(informServer(LOG_ON, generalSocket, &(address)) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to inform server on arrival");

    if(getClients(generalSocket, &(address), &rsrc) < 0)
        perror_free("dbclient: \e[31;1mfailed\e[0m to get dbox client list from server");
    close(generalSocket);
    bufferPrint(&(rsrc.buffer));
    listPrint(&(rsrc.list));

    // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ SERVE @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    // accept connections until an interrupt signal is caught
    while(1){
        fprintf(stdout, "\ndbclient: handling requests on port %hu(h)/%hu(n)\n", address.sin_port, htons(address.sin_port));

        // accept TCP connection
        if((generalSocket = accept(listeningSocket, NULL, NULL)) == -1){
            perror("dbclient: accepting connection \e[31;1mfailed\e[0m");
            continue;
        }

        // get code of request 
        else if(read(generalSocket, requestCode, FILE_CODE_LEN) != FILE_CODE_LEN){
            perror("dbclient: \e[31;1mfailed\e[0m to get request from client");
            continue;
        }
        requestCode[FILE_CODE_LEN-1] = '\0'; // ensure request string is terminated
        fprintf(stdout, "dbclient: received \e[1;93m'%s'\e[0m request\n", requestCode);

        // each request is followed by an (ip, port) pair
        if(getClientInfo(generalSocket, &peerInfo) < 0){
            perror("dbclient: \e[31;1mfailed\e[0m to get id from client");
            continue;
        }

        // resolve request
        if(handleRequest(argv[dirName], requestCode, generalSocket, &rsrc, &peerInfo) < 0)
                perror("dbclient: \e[31;1mfailed\e[0m to handle request");
        else
            fprintf(stdout, "dbclient: \e[32;1msuccess\e[0m");

        // close response socket, not to run out of file descriptors
        close(generalSocket);
    }
    getchar();
    handler(0);
    return 0; 
}

// in case of an interrupt signal
void handler(int sig){
    int i, lastSocket;
    uint8_t response;

    // inform workers of shut-down
    powerOn = 0;
    if(pthread_cond_broadcast(&(rsrc.emptyBuffer)) != 0 || pthread_cond_broadcast(&(rsrc.fullBuffer))){
        perror("dbclient: \e[31;1mfailed\e[0m to inform workers, leaving without them\n"); 
        free_rsrc(EXIT_FAILURE);
    }

    // let main thread join workers
    for(i = 0; i < workerThreads; i++)
        pthread_join(threadVector[i], NULL);

    // let server know that you are about to exit dbox system issuing a LOG_OFF request
    if((lastSocket = connectTo(&server)) < 0)
        perror("dbclient: \e[31;1mfailed\e[0m to establish a final connection before exiting");

    else{
        if(informServer(LOG_OFF, lastSocket, &(address)) < 0)
            perror("dbclient: \e[31;1mfailed\e[0m to inform server about LOG_OFF before exiting");
        else if(read(lastSocket, &response, sizeof(response)) != sizeof(response))
            perror("dbclient: \e[31;1mfailed\e[0m to inform server about LOG_OFF before exiting");
        else if(response == ERROR_IP_PORT_NOT_FOUND_IN_LIST)
                fprintf(stdout, "dbclient: got \e[31;1mERROR_IP_PORT_NOT_FOUND_IN_LIST\e[0m from server on exit\n");
        close(lastSocket);
    }

    free_rsrc(EXIT_SUCCESS);
}

// free all resources allocated for this client to operate
void free_rsrc(int exitCode){    
    close(listeningSocket);
    rsrcFree(&rsrc);
    free(threadVector);   
    free(mirror);
    exit(exitCode);
}

// print a usage message and exit with code: EXIT_FAILURE
void usage_error(const char *path){
    fprintf(stderr, "Usage: %s –d dirName –p portNum –w workerThreads –b bufferSize –sp serverPort –sip serverIP\n", path);
    exit(EXIT_FAILURE);
}

// display last error and exit after letting go of all resources used
void perror_free(char* msg){
    perror(msg);
    free_rsrc(EXIT_FAILURE);
}


