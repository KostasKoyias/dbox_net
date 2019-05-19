#include <stdio.h>
#include "include/clientInfo.h"
#include "include/list.h"
#include "include/define.h"

int main(int argc, char* argv[]){
    int i;
    struct clientInfo info;
    struct G_list clientlist = {NULL, sizeof(struct clientInfo), 0, clientCompare, clientAssign, clientPrint, NULL, NULL}; 



    listFree(&clientlist);
    return 0;
}