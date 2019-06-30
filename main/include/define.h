#ifndef define_H_
#define define_H_

#include <stdio.h>      
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>     //low-level I/O syscalls
#include <stdarg.h>      //variable argument lists 
#include <stdint.h>
#include <sys/file.h>    //flock
#include <unistd.h>      //fsync
#include <sys/socket.h>  //socket
#include <sys/stat.h>    //stat
#include <netinet/in.h>  //sockaddr_in 
#include <arpa/inet.h>   //inet_aton
#include <netdb.h>       //gethost*
#include <time.h>        //timespec,time_t
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>    //threads, mutexes
#include <dirent.h>     //readdir
#include <inttypes.h>   //uint types
#include <errno.h>      //perror

#define CLIENT_INSERT 0
#define CLIENT_DELETE 1
#define HOST_SIZE 256
#define PATH_SIZE 512
#define CODE_LEN 15         // requests on dbserver
#define FILE_CODE_LEN 20    // requests on main thread of dbclient
#define BACK_LOG 5
#define SOCKET_CAPACITY 1024
#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define FILE_PERMS 0644
#define DIR_PERMS 0755
#define EXIT_APPROVED 1
#define ERROR_IP_PORT_NOT_FOUND_IN_LIST 0
#define ERROR_IP_PORT_EXISTS -1
#endif