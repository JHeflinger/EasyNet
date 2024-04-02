#ifndef EASYSERVER
#define EASYSERVER

#include "netbasic.h"
#include "easystructs.h"

typedef struct {
    uint16_t port;
    EZN_SOCKET socket;
    EZN_SERVER_STATUS status;
    EZN_SERVER_TYPE type;
} ezn_Server;

typedef EZN_STATUS (*ezn_ServerBehavior)(ezn_Server*, EZN_SOCKET);

EZN_STATUS ezn_generate_server(ezn_Server* server, uint16_t port);
EZN_STATUS ezn_open_server(ezn_Server* server);
EZN_STATUS ezn_close_server(ezn_Server* server);
EZN_STATUS ezn_servers_are_open(EZN_BOOL* result);
EZN_STATUS ezn_clean_servers();
EZN_STATUS ezn_server_accept(ezn_Server* server, ezn_ServerBehavior behavior);
EZN_STATUS ezn_server_queue(ezn_Server* server, ezn_ServerBehavior behavior, int queuesize);

#endif
