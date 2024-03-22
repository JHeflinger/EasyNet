#pragma once
#include "netbasic.h"

EZN_STATUS ezn_hostname(char* name, size_t bufferlength);

#ifdef __linux__
EZN_STATUS ezn_hostname(char* name, size_t bufferlength) {
	return gethostname(name, bufferlength) == 0 ? EZN_NONE : EZN_ERROR;
}
#elif _WIN32
EZN_STATUS ezn_hostname(char* name, size_t bufferlength) {
	DWORD count = bufferlength;
	return GetComputerName(name, &count) == 1 ? EZN_NONE : EZN_ERROR;
}
#else
#error Unsupported operating system detected!
#endif