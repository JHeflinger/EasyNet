#pragma once

#define EZN_NONE 0
#define EZN_ERROR 1
typedef int EZN_STATUS;

#define MAX_IP_ADDR_LENGTH 64

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#elif _WIN32
#include <windows.h>
#else
#error Unsupported operating system detected!
#endif
