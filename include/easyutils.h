#pragma once
#include "netbasic.h"

EZN_STATUS ezn_set_ipv4_addr(uint8_t* address, uint8_t first, uint8_t second, uint8_t third, uint8_t fourth);
EZN_STATUS ezn_ipaddr_to_str(uint8_t* address, char* str, int strlen);

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
