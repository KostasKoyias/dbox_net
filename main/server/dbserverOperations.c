#include "../include/dbserverOperations.h"
#include "../include/utils.h"

// manage each possible client request
int handleRequest(char* requestCode, int responseSocket, struct G_list* clientlist){
        if(requestCode == NULL || clientlist == NULL)
            return -1;

        // if a client requested to log in, add client to the client list
        if(strcmp(requestCode, "LOG_ON") == 0)
            return clientsUpdate(CLIENT_INSERT, responseSocket, clientlist);

        // else if a client just asked for the client list, send it right away based on the protocol 
        else if(strcmp(requestCode, "GET_CLIENTS") == 0)
            return sendClients(responseSocket, clientlist);

        // else if a client just logged out, remove the client from our client list
        else if(strcmp(requestCode, "LOG_OFF") == 0)
            return clientsUpdate(CLIENT_DELETE, responseSocket, clientlist);

        // else the code passed is invalid
        else 
            return -1;

}

// send the whole client list via a socket to the client who made the request
int sendClients(int socket, struct G_list* list){
    struct G_node* iterator;
    struct clientInfo* info;

    // before sending the list, let the client know about the size of it
    if(write(socket , (int*)(&(list->length)), sizeof(int)) != sizeof(int))
        return -1;

    // send a (IP address, port) pair for each client in the list
    for(iterator = list->head; iterator != NULL; iterator = iterator->next){
        info = (struct clientInfo*)iterator->data;

        if(write(socket , &(info->ipAddress), sizeof(info->ipAddress)) != sizeof(info->ipAddress))
            return -2;
         if(write(socket , &(info->portNumber), sizeof(info->portNumber)) != sizeof(info->portNumber))
            return -3;
    }
    return 0;
}

// insert a new client or remove an existing one from the client list, based on a request code
int clientsUpdate(uint8_t operationCode, int socket, struct G_list* list){
    struct clientInfo info;
    char notFound[] = "ERROR_IP_PORT_NOT_FOUND_IN_LIST";
    if(list == NULL || socket < 0)
        return -1;

    // get ip address and port of client
    if(getClientInfo(socket, &info) != 0)
        return -2;

    // if a client logged in, add client to the client list and send USER ON to all other clients
    if(operationCode == CLIENT_INSERT){
        if(listInsert(list, &info) < 0)
            return -3;
        return informOtherClients(USER_ON, &info, list);
    }
    // else remove client from the client list, send USER OFF to all other clients
    else if(operationCode == CLIENT_DELETE){

        // if client does not exist, send a "ERROR_IP_PORT_NOT_FOUND_IN_LIST" message back to the client
        if(listDelete(list, &info) < 0){
            write(socket, notFound, sizeof(notFound));
            return -4;
        }
        return informOtherClients(USER_OFF, &info, list);
    }
    // else operation code is invalid
    else 
        return -5;
}

// when a user logs in or out, let all other clients know
int informOtherClients(uint8_t eventCode, struct clientInfo* info, struct G_list* list){
    char event[CODE_LEN];
    struct G_node* iterator;
    int generalSocket, rv = 0;
    extern int portNumber;
    struct sockaddr_in client, server = {.sin_port = portNumber, .sin_family = AF_INET};
    socklen_t clientlen;

    if(info == NULL || list == NULL)
        return -1;
    
    if(eventCode == USER_ON)
        strcpy(event, "USER_ON");
    else if(eventCode == USER_OFF)
        strcpy(event, "USER_OFF");

    // create a socket to use for all clients
    if((generalSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    // bind socket to your address, so that clients can recognise that it is the server trying to connect
    if(getMyIp(&(server.sin_addr)) < 0)
        return -2;
    if(bind(generalSocket, (struct sockaddr*)&server, sizeof(server)) < 0)
        return -3;

    
    // for each client in the list
    for(iterator = list->head; iterator != NULL; iterator = iterator->next){
        client.sin_addr.s_addr = info->ipAddress;
        client.sin_port = info->portNumber;

        // establish a connection with the client
        if(connect(generalSocket, (struct sockaddr*)&client, sizeof(client)) < 0){
            rv--;
            continue;
        }

        // inform client
        if((write(generalSocket, event, CODE_LEN) != CODE_LEN) 
            || write(generalSocket, &(info->ipAddress), sizeof(info->ipAddress))
            || write(generalSocket, &(info->portNumber), sizeof(info->portNumber))){
                rv--;
                continue;
        }

        // close and re-use socket
        close(generalSocket);
    }

    return 0;
}
