#include "../include/fd.h"

int fdAssign(void* fdA, const void* fdB){
    int *fd;
    if(fdA == NULL || fdB == NULL)
        return -1;
    fd = (int*)fdA;
    *fd = *((int*)fdB);
    return 0;
}

// compare 2 file descriptors
void* fdCompare(void* fdA, const void* fdB){
    if(fdA == NULL || fdB == NULL)
        return NULL;

    if(*((int*)fdA) == *((int*)fdB))
        return fdA;
    else
        return NULL;
}