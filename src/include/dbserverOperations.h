#ifndef dbserverOperations_H_
#define dbserverOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"

#define CLIENT_INSERT 0
#define CLIENT_DELETE 1

/*server operations*/
int clientlistOperation(uint8_t, int, struct G_list*);
int insertClient(int, struct G_list*);
int sendClients(int, struct G_list*);
int deleteClient(int, struct G_list*);

#endif