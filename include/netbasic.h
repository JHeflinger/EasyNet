#pragma once
#include "easylog.h"
#include <stdint.h>

#define EZN_NONE 0
#define EZN_ERROR 1
typedef uint32_t EZN_STATUS;

#define EZN_FALSE 0
#define EZN_TRUE 1
typedef uint8_t EZN_BOOL;

#define MAX_IP_ADDR_LENGTH 64
#define MIN_PORT 1024
#define MAX_PORT 65535
#define DEFAULT_PORT 44448

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <arpa/inet.h>
#elif _WIN32
#include <winsock2.h>
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