#ifndef EASYCLIENT
#define EASYCLIENT

#include "netbasic.h"
#include "easystructs.h"
#include "easyutils.h"

typedef struct {
	uint8_t address[IPV4_ADDR_LENGTH];
	uint16_t port;
	EZN_SOCKET socket;
	EZN_CLIENT_STATUS status;
	EZN_CLIENT_TYPE type;
} ezn_Client;

typedef EZN_STATUS (*ezn_ClientBehavior)(ezn_Client*, EZN_SOCKET);

EZN_STATUS ezn_configure_client(ezn_Client* client, uint16_t port, uint8_t* address);
EZN_STATUS ezn_connect_client(ezn_Client* client, ezn_ClientBehavior behavior);
EZN_STATUS ezn_disconnect_client(ezn_Client* client);
EZN_STATUS ezn_clients_are_open(EZN_BOOL* result);
EZN_STATUS ezn_clean_clients();

#endif
