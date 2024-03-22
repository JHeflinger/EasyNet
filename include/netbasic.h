#pragma once

#define EZN_NONE 0
#define EZN_ERROR 1
typedef int EZN_STATUS;

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#elif _WIN32
#include <windows.h>
#else
#error Unsupported operating system detected!
#endif
