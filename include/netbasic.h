#pragma once

#define EN_NONE 0
#define EN_ERROR 1
typedef int EN_STATUS;

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#endif
