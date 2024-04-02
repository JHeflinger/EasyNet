#include "netbasic.h"

static int s_is_initialized = 0;

int ezn_is_initialized() {
	return s_is_initialized;
}

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
