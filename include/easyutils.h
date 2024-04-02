#ifndef EASYUTILS
#define EASYUTILS

#include "netbasic.h"

EZN_STATUS ezn_set_ipv4_addr(uint8_t* address, uint8_t first, uint8_t second, uint8_t third, uint8_t fourth);
EZN_STATUS ezn_ipaddr_to_str(uint8_t* address, char* str, int strlen);
EZN_STATUS ezn_str_to_ipaddr(uint8_t* address, char* str);
EZN_STATUS ezn_str_to_port(uint16_t* port, char* str);

#endif
