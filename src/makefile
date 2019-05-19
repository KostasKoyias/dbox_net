CC = gcc
CFLAGS = -g3 -Wall
HDRS = define.h utils.h 
OBJS = utils.o clientInfo.o list.o dbserverOperations.o
LFLAGS = -lpthread -lrt
SOURCES = utils.c clientInfo.c list.c dbserverOperations.c

all: dbserver dbclient $(OBJS)  

# applications
dbserver: dbserver.c $(OBJS)
	$(CC) $(CFLAGS) -o dbserver dbserver.c $(SOURCES) $(LFLAGS)

dbclient: dbclient.c $(OBJS)
	$(CC) $(CFLAGS) -o dbclient dbclient.c $(SOURCES) $(LFLAGS)

# data structures and utilities	
utils.o: utils.c 
	$(CC) $(CFLAGS) -c utils.c $(LFLAGS)

list.o: list.c 
	$(CC) $(CFLAGS) -c list.c $(LFLAGS)

clientInfo.o: clientInfo.c 
	$(CC) $(CFLAGS) -c clientInfo.c $(LFLAGS)

dbserverOperations.o: dbserverOperations.c
	$(CC) $(CFLAGS) -c dbserverOperations.c $(LFLAGS)

clean: 
	rm -rf $(OBJS) dbserver dbclient
 