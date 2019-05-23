#include "../include/circularBuffer.h"

// copy all information about a file to another
int fileAssign(struct fileInfo* fileA, const struct fileInfo* fileB){
    if(fileA == NULL || fileB == NULL)
        return -1;

    strcpy(fileA->path, fileB->path);
    fileA->version = fileB->version;
    return clientAssign(&(fileA->owner), &(fileB->owner));
}

// given a circular buffer and the size of it, initialize all fields appropriately 
int bufferInit(int bufferSize, struct circularBuffer* buffer){
    int i;
    if(buffer == NULL)
        return -1;

    // initialize buffer pointers
    buffer->start = -1;
    buffer->end = 0;
    buffer->size = bufferSize;

    // allocate space for 'bufferSize' file meta-data structures
    if((buffer->files = malloc(sizeof(struct fileInfo) * bufferSize)) == NULL)
        return -2;

    // initialize each space in the buffer as empty
    for(i = 0; i < buffer->size; i++)
        buffer->files[i].version = -2;    
    return 0;
}

// add an item to the end of the buffer, if not full, if "last" place is taken, it will be placed at index 0
int bufferAdd(const struct fileInfo* file, struct circularBuffer* buffer){
    if(file == NULL || buffer == NULL || buffer->size <= 0)
        return -1;

    // if this is the first item ever inserted in the buffer, set start pointer at index 0
    if(buffer->start == -1)
        buffer->start = 0;

    // else if buffer is full, do nothing
    else if(bufferIsFull(buffer))
        return -2;
    
    // add this item in the first position available, update the pointer to the now next position available 
    if(fileAssign(&(buffer->files[buffer->end]), file) < 0)
        return -3;
    buffer->end = (buffer->end + 1) % (buffer->size); 
    return 0;

}

// remove an item from the circular buffer, if not empty
int bufferRemove(struct fileInfo* file, struct circularBuffer* buffer){
    if(file == NULL || buffer == NULL || buffer->size <= 0)
        return -1;
    
    // if buffer is empty, skip
    if(bufferIsEmpty(buffer))
        return -2;

    // insert file data into the address passed 
    else if(fileAssign(file, &(buffer->files[buffer->start])) < 0)
        return -3;

    // mark file as deleted, so that it can be overwritten and update 'next to pop' file pointer
    buffer->files[buffer->start].version = -2;
    buffer->start = (buffer->start + 1) % (buffer->size);
    return 0;
}

// if the next position to insert at is taken return 0, else 1
int bufferIsFull(struct circularBuffer* buffer){
    if(buffer == NULL)
        return -1;

    return buffer->files[buffer->end].version != -2;
}

// if no element has ever been added(start = -1) or the space of the starting element is empty, then buffer is empty
int bufferIsEmpty(struct circularBuffer* buffer){
    if(buffer == NULL)
        return -1;
    else 
        return buffer->start == -1 ? 1 : buffer->files[buffer->start].version == -2;
}

// display contents of a circular buffer
int bufferPrint(struct circularBuffer* buffer){
    int i;
    if(buffer == NULL)
        return -1;
        
    fprintf(stdout, "\e[1;4;35mBuffer\e[0m\nstart:\t%d\nend:\t%d\n", buffer->start, buffer->end);
    for(i = 0; i < buffer->size ; i = i + 1)
        if(buffer->files[i].version == -2)
            fprintf(stdout, " | (-, -) | ");
        else if(buffer->files[i].version == -1)
            fprintf(stdout, " | (%u, %hu) | ", buffer->files[i].owner.ipAddress, buffer->files[i].owner.portNumber);
        else
            fprintf(stdout, "| (ip:%u, p:%hu, v:%d, %s) |",\
            buffer->files[i].owner.ipAddress, buffer->files[i].owner.portNumber, buffer->files[i].version, buffer->files[i].path);
    fputc('\n', stdout);
    return 0;
}

// free all resources allocated for a circular buffer
int bufferFree(struct circularBuffer* buffer){
    if(buffer == NULL)
        return -1;
    free(buffer->files);
    return 0;
}