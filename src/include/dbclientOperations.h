#ifndef dbclientOperations_H_
#define dbclientOperations_H_
#include "define.h"
#include "list.h"
#include "clientInfo.h"
#include "utils.h"

#define LOG_ON 0
#define LOG_OFF 1

/*client operations*/
int informServer(uint8_t, int, struct sockaddr_in*);
int getClients(int, struct G_list*);

#endif