CC = gcc
BASE = $(shell pwd)
CFLAGS = -g3 -Wall -I$(BASE)/include
LFLAGS = -lpthread -lrt
SOURCES = utils/netUtils.c utils/msgUtils.c utils/list.c utils/circularBuffer.c utils/clientInfo.c utils/utils.c 
RSRC = list.o clientInfo.o circularBuffer.o msgUtils.o netUtils.o utils.o 
SERVER = dbserverOperations.o 
CLIENT = dbclientOperations.o dbclientRequests.o dbclientInit.o dbclientWorker.o
SERVER_SRCS = server/dbserver.c server/dbserverOperations.c
CLIENT_SRCS = client/dbclient.c client/dbclientOperations.c client/dbclientRequests.c client/dbclientInit.c client/dbclientWorker.c
OBJS = $(RSRC) $(SERVER) $(CLIENT)

all: dbserver dbclient $(OBJS) 

# applications
dbserver: $(SERVER_SRCS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $? $(LFLAGS)

dbclient: $(CLIENT_SRCS) $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $? $(LFLAGS)

# utilities and data structures	
msgUtils.o: utils/msgUtils.c 
	$(CC) $(CFLAGS) -c utils/msgUtils.c $(LFLAGS)

netUtils.o: utils/netUtils.c 
	$(CC) $(CFLAGS) -c utils/netUtils.c $(LFLAGS)

utils.o: utils/utils.c 
	$(CC) $(CFLAGS) -c utils/utils.c $(LFLAGS)

list.o: utils/list.c 
	$(CC) $(CFLAGS) -c utils/list.c $(LFLAGS)

clientInfo.o: utils/clientInfo.c 
	$(CC) $(CFLAGS) -c utils/clientInfo.c $(LFLAGS)

circularBuffer.o: utils/circularBuffer.c
	$(CC) $(CFLAGS) -c utils/circularBuffer.c $(LFLAGS)

# server
dbserverOperations.o: server/dbserverOperations.c
	$(CC) $(CFLAGS) -c server/dbserverOperations.c $(LFLAGS)

# client
# 		main thread
dbclientOperations.o: client/dbclientOperations.c
	$(CC) $(CFLAGS) -c client/dbclientOperations.c $(LFLAGS)
dbclientRequests.o: client/dbclientRequests.c
	$(CC) $(CFLAGS) -c client/dbclientRequests.c $(LFLAGS)
dbclientInit.o: client/dbclientInit.c
	$(CC) $(CFLAGS) -c client/dbclientInit.c $(LFLAGS)

#		worker thread
dbclientWorker.o: client/dbclientWorker.c
	$(CC) $(CFLAGS) -c client/dbclientWorker.c $(LFLAGS)

clean: 
	rm -rf $(OBJS) dbserver dbclient
 