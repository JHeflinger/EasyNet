#ifndef EASYSTRUCTS
#define EASYSTRUCTS

#include "netbasic.h"

struct ezn_SocketList{
    uint32_t socket;
    struct ezn_SocketList* next;
};

typedef struct ezn_SocketList ezn_SocketList;

void ezn_free_socketlist(ezn_SocketList* socketlist);

#endif
