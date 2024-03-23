#include "easynet.h"

EZN_STATUS server_behavior(ezn_Server* server, uint32_t clientsock) {

	return EZN_NONE;
}

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

	ezn_Server server;
	status = ezn_generate_server(&server, 43367);
	if (status == EZN_NONE) {
		EZN_INFO("Successfully generated server device for port %d", server.port);
	} else {
		EZN_FATAL("Unable to generate server device");
	}
	
	status = ezn_open_server(&server);
	if (status == EZN_NONE) {
		EZN_INFO("Successfully opened server on port %d", server.port);
	} else {
		EZN_FATAL("Unable to open server");
	}

	status = ezn_server_accept(&server, server_behavior);
	if (status == EZN_NONE) {
		EZN_INFO("Finished taking clients");
	} else {
		EZN_WARN("Unable to properly execute client behavior");
	}
	 //   |
	//	  -> server.branch(functionptr);
	// server.wait();

	status = ezn_close_server(&server);
	if (status == EZN_NONE) {
		EZN_INFO("Successfully closed server on port %d", server.port);
	} else {
		EZN_FATAL("Unable to close server");
	}

	status = ezn_safe_clean();
	if (status == EZN_ERROR) {
		EZN_WARN("Unable to successfully perform a safe clean");
	}
	ezn_clean();
	return 0;
}
