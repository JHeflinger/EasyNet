#include "easynet.h"
#include "easylog.h"

int main(int argc, char* argv[]) {
	ezn_init();

	char name[MAX_HOST_NAME_LENGTH];
	char addr[MAX_IP_ADDR_LENGTH];

	EZN_STATUS status = ezn_hostname(name, MAX_HOST_NAME_LENGTH);
	if (status == EZN_NONE) {
		EZN_INFO("The hostname is %s", name);
	} else {
		EZN_FATAL("Hostname could not be detected, internal error found");
	}

	status = ezn_hostaddress(addr, MAX_HOST_NAME_LENGTH);
	if (status == EZN_NONE) {
		EZN_INFO("The host ip is %s", addr);
	} else {
		EZN_FATAL("Host IP could not be detected, internal error found");
	}

	ezn_clean();
	return 0;
}
