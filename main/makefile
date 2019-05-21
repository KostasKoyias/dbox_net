CC = gcc
CFLAGS = -g3 #-Wall
LFLAGS = -lpthread -lrt
SOURCES = utils/utils.c utils/list.c utils/circularBuffer.c utils/clientInfo.c
RSRC = list.o clientInfo.o circularBuffer.o utils.o 
SERVER = dbserverOperations.o 
CLIENT = dbclientOperations.o dbclientRequests.o dbclientInit.o dbclientWorker.o
OBJS = $(RSRC) $(SERVER) $(CLIENT)

all: dbserver dbclient $(OBJS) 

# applications
dbserver: server/dbserver.c
	$(CC) $(CFLAGS) -o dbserver server/*.c $(SOURCES) $(LFLAGS)

dbclient: client/dbclient.c $(RESOURCES)
	$(CC) $(CFLAGS) -o dbclient client/*.c $(SOURCES) $(LFLAGS)

# utilities	
utils.o: utils/utils.c 
	$(CC) $(CFLAGS) -c utils/utils.c $(LFLAGS)

# data structures and resources
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
 