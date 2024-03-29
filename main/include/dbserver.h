#ifndef dbserverOperations_H_
#define dbserverOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"

#define USER_ON 0
#define USER_OFF 1

/*server operations*/
int clientsUpdate(uint8_t, int, struct G_list*);
int sendClients(int, struct G_list*);
int informOtherClients(uint8_t, struct clientInfo*, struct G_list*);
int handleRequest(char*, int, struct G_list*);

#endif