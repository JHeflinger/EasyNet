#ifndef EASYPACKET
#define EASYPACKET

#include "netbasic.h"

EZN_STATUS ezn_recieve(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength);
EZN_STATUS ezn_ask(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength);
EZN_STATUS ezn_send(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* sentlength);

#endif
