#include "hostinfo.h"

#ifdef __linux__

EZN_STATUS ezn_hostname(char* name, const size_t bufferlength) {
	EZN_SAFECHECK();
	return gethostname(name, bufferlength) == 0 ? EZN_NONE : EZN_ERROR;
}

EZN_STATUS ezn_hostaddress(char* address, const size_t bufferlength) {
	EZN_SAFECHECK();
	if (bufferlength < MAX_IP_ADDR_LENGTH) {
		EZN_WARN("Rejecting request for host address, given return buffer is not big enough. Consider using MAX_IP_ADDR_LENGTH.");
		return EZN_ERROR;
	}
	char hostname[MAX_HOST_NAME_LENGTH];
	if (ezn_hostname(hostname, MAX_HOST_NAME_LENGTH) == EZN_ERROR) return EZN_ERROR;
	struct hostent* hostentry = gethostbyname(hostname);
	if (hostentry == NULL) return EZN_ERROR;
	strcpy(address, inet_ntoa(*((struct in_addr*)hostentry->h_addr_list[0])));
	return EZN_NONE;
}

#elif _WIN32

EZN_STATUS ezn_hostname(char* name, size_t bufferlength) {
	EZN_SAFECHECK();
	DWORD count = bufferlength;
	return GetComputerName(name, &count) == 1 ? EZN_NONE : EZN_ERROR;
}

EZN_STATUS ezn_hostaddress(char* address, const size_t bufferlength) {
	EZN_SAFECHECK();
	if (bufferlength < MAX_IP_ADDR_LENGTH) {
		EZN_WARN("Rejecting request for host address, given return buffer is not big enough. Consider using MAX_IP_ADDR_LENGTH.");
		return EZN_ERROR;
	}
	char hostname[MAX_HOST_NAME_LENGTH];
	if (ezn_hostname(hostname, MAX_HOST_NAME_LENGTH) == EZN_ERROR) return EZN_ERROR;
	struct hostent* hostentry = gethostbyname(hostname);
	if (hostentry == NULL) return EZN_ERROR;
	strcpy(address, inet_ntoa(*((struct in_addr*)hostentry->h_addr_list[0])));
	return EZN_NONE;
}

#else
#error Unsupported operating system detected!
#endif
