#pragma once
#include "netbasic.h"
#include <stdint.h>

struct ezn_SocketList{
    uint32_t socket;
    struct ezn_SocketList* next;
};

typedef struct ezn_SocketList ezn_SocketList;

void ezn_free_socketlist(ezn_SocketList* socketlist) {
    ezn_SocketList* curr = socketlist;
    while (curr->next != NULL) {
        ezn_SocketList* next = curr->next;
        free(curr);
        curr = next;
    }
    free(curr);
}