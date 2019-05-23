#ifndef circularBuffer_H_
#define circularBuffer_H_
#include "define.h"
#include "clientInfo.h"
#define SEQ_LEN 10

// fileInfo holding all necessary meta-data for dbox to work 
struct fileInfo{
    char path[PATH_SIZE];
    int version;
    struct clientInfo owner;
};

int fileAssign(struct fileInfo*, const struct fileInfo*);

// a circular buffer holding file information for a series of files 
struct circularBuffer{
    int start; // first reserved position
    int end;   // first empty position
    uint8_t size;
    struct fileInfo* files;
};


// circular buffer member methods
int bufferInit(int, struct circularBuffer*);
int bufferAdd(const struct fileInfo*, struct circularBuffer*);
int bufferRemove(struct fileInfo*, struct circularBuffer*);
int bufferIsFull(struct circularBuffer*);
int bufferIsEmpty(struct circularBuffer*);
int bufferPrint(struct circularBuffer*);
int bufferFree();

#endif