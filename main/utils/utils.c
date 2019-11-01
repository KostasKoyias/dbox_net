/*******************************************************************************************\
 * The following are some short utility functions of all kinds, making code re-use simpler *
\*******************************************************************************************/
#include "utils.h"


// replace and put result in dst, e.g src = d0/d1/d2 --> dst = d0/d1/new
int setPath(char* dst, const char* src, const char* new){
    int index;
    if(src == NULL || dst == NULL || new == NULL)
        return -1;

    // all path until a leaf in dst is same as source
    if((index = lastIndexOf('/', src)) != -1)
        snprintf(dst, index + 2, "%s/", src);

    // rest is the new file/directory
    strcpy(dst + index + 1, new);
    return 0;
}

// get last index of a character in a string, or -1 if not found
int lastIndexOf(char c, const char* str){
    int i;
    if(str == NULL)
        return -1;

    for(i = strlen(str)-2; i >= 0; i--){
        if(str[i] == c)
            return i;
    }
    return -1;
}

// given a path of a regular file, make parent directories as needed
int makeParents(char* path, mode_t mode){
    int i, rv = 0;
    char temp;
    if(path == NULL || (i = lastIndexOf('/', path)) < 0)
        return -1;
    temp = path[i];
    path[i] = '\0';
    rv = mkdirTree(path, mode);
    path[i] = temp;
    return rv;
}


// given a directory path, create it making parent directories as needed, similar to --parents option of mkdir system program 
int mkdirTree(char* path, mode_t mode){
    int i;
    char temp;
    if(path == NULL)
        return -1;

    // for each / encountered, create directory up to this point
    for(i = 0; i < strlen(path); i++){
        if(path[i] == '/'){
            temp = path[i];
            path[i] = '\0';
            if(mkdir(path, mode) < 0 && errno != EEXIST){
                printf("mkdir %s = failI\n", path);
                perror("err: ");
                return -2;
            }
            path[i] = temp;
        }
    }

    // create last directory
    if(mkdir(path, mode) < 0){
        if(errno == EEXIST)
            return EEXIST;
        else{
            printf("mkdir %s = fail lastII\n", path);
            perror("err: ");

            return -3;
        }
    }
    return 0;
}

// check whether a file exists on this file system and return the lastest version id
int statFile(char* path){
    struct stat statBuffer;

    // get status of file, return FILE_NOT_FOUND in case of failure
    if(stat(path, &statBuffer) == -1)
        return -1;
    else 
        return (int)statBuffer.st_mtime;
}

// given an integer, return the number of digits it has
int digits(int number){
    int digits = 0;
    while(number != 0){
        number /= 10;
        digits++;
    }
    return digits;
}

// turn a linked list of integers into a socket set
int socketSetInit(int listeningSocket, struct G_list *vector, fd_set *set){
    struct G_node *i;
    if(vector == NULL || set == NULL)
        return -1;

    // clear set, then add the listening socket and all socket descriptors of vector into the socket set
    FD_ZERO(set);
    FD_SET(listeningSocket, set);
    for(i = vector->head ; i != NULL; i = i->next)
        FD_SET(*((int*)i->data), set);
    return 0;
}

// find out which socket forced select to return
int getActiveSocket(struct G_list *vector, fd_set *set){
    struct G_node *i;
    int fd;
    if(vector == NULL || set== NULL)
        return -1;
    
    for(i = vector->head ; i != NULL; i = i->next){
        fd = *((int*)i->data);
        if(FD_ISSET(fd, set))
            return fd;
    }
    return -2;
}

// given a file descriptor of a listening socket and a list of fds, return the one with having the maximum value 
int getMaxFd(int listeningSocket, struct G_list* list){
    struct G_node* i;
    int max = listeningSocket;
    if(list == NULL)
        return listeningSocket;

    for(i = list->head; i != NULL; i = i->next)
        max = MAX(listeningSocket, *((int *)i->data));
    return max;
}