#ifndef HOSTINFO
#define HOSTINFO

#include "netbasic.h"
#include "easylog.h"

#define MAX_HOST_NAME_LENGTH 253

EZN_STATUS ezn_hostname(char* name, const size_t bufferlength);
EZN_STATUS ezn_hostaddress(char* address, const size_t bufferlength);

#endif
