CC = gcc
CFLAGS = -g3 -Wall
LFLAGS = -lpthread -lrt
RESOURCES = utils/* rsrc/*

all: dbserver dbclient   

# applications
dbserver: server/dbserver.c 
	$(CC) $(CFLAGS) -o dbserver server/* $(RESOURCES) $(LFLAGS)

dbclient: client/dbclient.c
	$(CC) $(CFLAGS) -o dbclient client/* $(RESOURCES) $(LFLAGS)

clean: 
	rm -rf dbserver dbclient
 