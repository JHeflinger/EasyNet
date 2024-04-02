#include "easyutils.h"

EZN_STATUS ezn_set_ipv4_addr(uint8_t* address, uint8_t first, uint8_t second, uint8_t third, uint8_t fourth) {
	address[0] = first;
	address[1] = second;
	address[2] = third;
	address[3] = fourth;
	return EZN_NONE;
}

EZN_STATUS ezn_ipaddr_to_str(uint8_t* address, char* str, int strlen) {
	if (strlen < 17) {
		EZN_WARN("given string length is not long enough");
		return EZN_ERROR;
	}
	sprintf(str, "%d.%d.%d.%d", address[0], address[1], address[2], address[3]);
	return EZN_NONE;
}

EZN_STATUS ezn_str_to_ipaddr(uint8_t* address, char* str) {
	int ind = 0;
	int addrind = 0;
	int found = 0;
	int inter = 0;
	while (str[ind] != '\0') {
		if (found > 3) return EZN_ERROR;
		if (str[ind] == '.') {
			if (inter > 255) return EZN_ERROR;
			address[addrind] = inter;
			addrind++;
			found++;
			inter = 0;
		} else if (str[ind] >= '0' && str[ind] <= '9') {
			inter *= 10;
			inter += str[ind] - '0';
		} else {
			return EZN_ERROR;
		}
		ind++;
	}
	if (found < 3 || inter > 255) {
		return EZN_ERROR;
	}
	address[3] = inter;
	return EZN_NONE;
}

EZN_STATUS ezn_str_to_port(uint16_t* port, char* str) {
	int ind = 0;
	uint64_t inter = 0;
	while(str[ind] != '\0') {
		if (str[ind] >= '0' && str[ind] <= '9') {
			inter *= 10;
			inter += str[ind] - '0';
			if (inter > MAX_PORT) return EZN_ERROR;
		} else return EZN_ERROR;
		ind++;
	}
	if (inter > MAX_PORT) return EZN_ERROR;
	*port = inter;
	return EZN_NONE;
}
