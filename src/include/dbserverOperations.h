#ifndef dbserverOperations_H_
#define dbserverOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"

#define CLIENT_INSERT 0
#define CLIENT_DELETE 1

/*server operations*/
int clientlistUpdate(uint8_t, int, struct G_list*);
int sendClients(int, struct G_list*);

#endif