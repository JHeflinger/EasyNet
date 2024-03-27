#pragma once
#include "netbasic.h"

EZN_STATUS ezn_recieve(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength);
EZN_STATUS ezn_ask(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength);
EZN_STATUS ezn_send(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* sentlength);

EZN_STATUS ezn_recieve(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength) {
	EZN_SAFECHECK();
    *returnlength = recv(clientsocket, (char*)buffer, bufflength, 0);
    if (*returnlength < 0) {
        EZN_WARN("An error occured while recieving data");
        return EZN_ERROR;
    }
    return EZN_NONE;
}

EZN_STATUS ezn_ask(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* returnlength) {
	EZN_SAFECHECK();
    
    #ifdef _WIN32
    unsigned long l;
    ioctlsocket(clientsocket, FIONREAD, &l);
    if (l > 0) *returnlength = recv(clientsocket, (char*)buffer, bufflength, 0);
    else *returnlength = 0;
    #elif __linux__	
	ssize_t retval = recv(clientsocket, (char*)buffer, bufflength, EZN_DONT_WAIT);
	if (retval < 0) retval = 0;
	*returnlength = retval;
	#else
    #error "Unsupported operating system detected!
    #endif

    if (*returnlength < 0) {
        EZN_WARN("An error occured while recieving data");
        return EZN_ERROR;
    }
    return EZN_NONE;
}

EZN_STATUS ezn_send(EZN_SOCKET clientsocket, EZN_BYTE* buffer, size_t bufflength, size_t* sentlength) {
	EZN_SAFECHECK();
    *sentlength = send(clientsocket, (char*)buffer, bufflength, 0);
    if (*sentlength < 0) {
        EZN_WARN("An error occured while sending data");
        return EZN_ERROR;
    }
    return EZN_NONE;
}
