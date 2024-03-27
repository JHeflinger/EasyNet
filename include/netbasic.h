#pragma once
#include "easylog.h"
#include <stdint.h>

#define EZN_NONE 0
#define EZN_ERROR 1
typedef uint32_t EZN_STATUS;

#define EZN_FALSE 0
#define EZN_TRUE 1
typedef uint8_t EZN_BOOL;

#define EZN_TCP 0
#define EZN_UDP 1
typedef int EZN_SERVER_TYPE;
typedef int EZN_CLIENT_TYPE;

#define EZN_SERVER_OPEN 0
#define EZN_SERVER_CLOSED 1
typedef int EZN_SERVER_STATUS;

#define EZN_CLIENT_CONNECTED 0
#define EZN_CLIENT_DISCONNECTED 1 
typedef int EZN_CLIENT_STATUS;

typedef uint8_t EZN_BYTE;

#define MAX_IP_ADDR_LENGTH 64
#define MIN_PORT 1024
#define MAX_PORT 65535
#define DEFAULT_PORT 44448

#define IPV4_ADDR_LENGTH 4

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <pthread.h>

#define EZN_INVALID_SOCK -1
#define EZN_PROTOCOL int
#define EZN_TCP_PROTOCOL 0
#define EZN_UDP_PROTOCOL 0
#define EZN_CLOSE(...) close(__VA_ARGS__)
#define EZN_OPT_TYPE int
#define EZN_DONT_WAIT MSG_DONTWAIT

typedef int EZN_SOCKET;

#elif _WIN32

#include <winsock2.h>

#define EZN_INVALID_SOCK INVALID_SOCKET
#define EZN_PROTOCOL IPPROTO
#define EZN_TCP_PROTOCOL IPPROTO_TCP
#define EZN_UDP_PROTOCOL IPPROTO_UDP
#define EZN_CLOSE(...) closesocket(__VA_ARGS__)
#define EZN_OPT_TYPE char
#define EZN_DONT_WAIT 0

typedef SOCKET EZN_SOCKET;

#else
#error Unsupported operating system detected!
#endif

static int s_is_initialized = 0;

void ezn_init();
void ezn_clean();

#ifdef __linux__

void ezn_init() {
    s_is_initialized = 1;
}

void ezn_clean() {
    s_is_initialized = 0;
}

#elif _WIN32

void ezn_init() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        EZN_FATAL("Failed to initialize Winsock.\n");
    }
    s_is_initialized = 1;
}

void ezn_clean() {
    WSACleanup();
    s_is_initialized = 0;
}

#else
#error Unsupported operating system detected!
#endif

#define EZN_SAFECHECK() {if (!s_is_initialized) { EZN_WARN("EasyNet was not intialized. Please initialize it using ezn_init(). Implicitly intializing now..."); ezn_init();}}
