#include "../include/dbserver.h"
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
    uint8_t response[3] = {-1, 0, 1};
    if(list == NULL || socket < 0)
        return -1;

    // get ip address and port of client
    if(getClientInfo(socket, &info) != 0)
        return -2;

    // if a client logged in, add client to the client list and send USER ON to all other clients
    if(operationCode == CLIENT_INSERT){

        // if client exists, respond with code -1 indicating ERROR_IP_PORT_EXISTS, else send list of connected clients to the new one
        if(listInsert(list, &info) < 0){
            if(write(socket, &(response[0]), sizeof(response[0])) != sizeof(response[0]))
                return -3;
            return 1;
        }
        if(sendClients(socket, list) < 0)
            return -4;
        return informOtherClients(USER_ON, &info, list);
    }
    // else remove client from the client list, send USER OFF to all other clients
    else if(operationCode == CLIENT_DELETE){

        // if client does not exist, respond with a 0 indicating: "ERROR_IP_PORT_NOT_FOUND_IN_LIST", else respond sending 1
        if(listDelete(list, &info) < 0){
            if(write(socket, &(response[1]), sizeof(response[1])) != sizeof(response[1]))
                return -5;
            return 2;
        }
        if(write(socket, &(response[2]), sizeof(response[2])) != sizeof(response[2]))
            return -6;
        return informOtherClients(USER_OFF, &info, list);
    }
    // else operation code is invalid
    else 
        return -7;
}

// when a user logs in or out, let all other clients know
int informOtherClients(uint8_t eventCode, struct clientInfo* info, struct G_list* list){
    char event[FILE_CODE_LEN];
    struct G_node* iterator;
    int generalSocket, informed = 0;
    struct sockaddr_in client = {.sin_family = AF_INET};

    if(info == NULL || list == NULL)
        return -1;
    
    if(eventCode == USER_ON)
        strcpy(event, "USER_ON");
    else if(eventCode == USER_OFF)
        strcpy(event, "USER_OFF");

    // for each client in the list
    for(iterator = list->head; iterator != NULL; iterator = iterator->next, close(generalSocket)){// close connection
        client.sin_addr.s_addr = ((struct clientInfo*)iterator->data)->ipAddress;
        client.sin_port = ((struct clientInfo*)iterator->data)->portNumber;

        // omit the client who triggered this event, in case of an insertion
        if(eventCode == USER_ON && client.sin_addr.s_addr == info->ipAddress && client.sin_port == info->portNumber)
            continue;

        // establish a connection with the client
        if((generalSocket = connectTo(&client)) < 0){
            perror("connect");
            continue;
        }

        // inform client specifying the user that entered/left the system
        if(write(generalSocket, event, FILE_CODE_LEN) != FILE_CODE_LEN){ perror("write");
            continue;}
        else if(sendClientInfo(generalSocket, info) < 0)
            continue;
        informed++;
    }

    // if there was even one client not informed properly, excluding the one who triggered the event, return -1 else 0
    return (informed == list->length - (eventCode == USER_ON)) - 1;
}
