#include "easystructs.h"

void ezn_free_socketlist(ezn_SocketList* socketlist) {
    ezn_SocketList* curr = socketlist;
    while (curr->next != NULL) {
        ezn_SocketList* next = curr->next;
        free(curr);
        curr = next;
    }
    free(curr);
}
