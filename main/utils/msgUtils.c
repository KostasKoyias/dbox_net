/***********************************************************************************************\
 * The following are some short utility functions, making code re-use simpler                   *
 * most of them handle common error cases and code that is widely used through out this project *
\***********************************************************************************************/
#include "../include/utils.h"

// print error indicated by errno and exit with code 1(FAILURE)
void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

// print a message in stderr and exit with code 1(FAILURE)
void error_exit(const char* format,...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

// print a message in stderr and return $ret_val to the caller
int error_return(int ret_val, const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    return ret_val;
}

// lock a file and print a message in it
int exclusive_print(int fd, const char* format, ...){

    // get arguments
    va_list args;
    va_start(args, format);

    // lock file,print message and flush to the disc
    flock(fd, LOCK_EX);
    vdprintf(fd, format, args);
    fsync(fd);
    flock(fd, LOCK_UN);
    va_end(args);
    return 0;
}

// replace and put result in dst, e.g src = d0/d1/d2 --> dst = d0/d1/new
int setPath(char* dst, const char* src, const char* new){
    int index;
    if(src == NULL || dst == NULL)
        return -1;

    // all path until a leaf in dst is same as source
    if((index = lastIndexOf('/', src)) != -1)
        snprintf(dst, index + 2, "%s/", src);

    // rest is the new file/directory
    strcpy(dst + index + 1, new);
}

// get last index of a character in a string, or -1 if not found
int lastIndexOf(char c, const char* str){
    int i;
    if(str == NULL)
        return -1;

    for(i = strlen(str)-1; i >= 0; i--){
        if(str[i] == c)
            return i;
    }
    return -1;
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
            if(mkdir(path, mode) < 0 && errno != EEXIST)
                return -2;
            path[i] = temp;
        }
    }

    // create last directory
    if(mkdir(path, mode) < 0){
        if(errno == EEXIST)
            return EEXIST;
    }
    return 0;
}


