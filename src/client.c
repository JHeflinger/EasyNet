#include "easynet.h"

EZN_STATUS client_behavior(ezn_Client* client, EZN_SOCKET serversock) {
	char buffer[1024];
	EZN_SCAN("%[^\n]", buffer);
	size_t returnlen;
	if (ezn_send(serversock, buffer, strlen(buffer), &returnlen) == EZN_ERROR) {
		EZN_WARN("send failed");
		return EZN_ERROR;
	}
	EZN_INFO("Sent message: %s", buffer);
	return EZN_NONE;
}

int main(int argc, char* argv[]) {
	ezn_init();

	ezn_Client client;
	uint8_t address[IPV4_ADDR_LENGTH];
	ezn_set_ipv4_addr(address, 127, 0, 0, 1);
	EZN_STATUS status;

	char addrstr[1024];
	ezn_ipaddr_to_str(address, addrstr, 1024);
	EZN_INFO("Connecting client to %s on port %d", addrstr, DEFAULT_PORT);

	status = ezn_configure_client(&client, DEFAULT_PORT, address);
	if (status == EZN_NONE) {
		EZN_INFO("Successfully configured client");
	} else {
		EZN_FATAL("An error occurred while configuring the client");
	}

	status = ezn_connect_client(&client, client_behavior);
	if (status == EZN_NONE) {
		EZN_INFO("Finished talking to server!");
	} else {
		EZN_WARN("Unable to properly connect client");
	}

	status = ezn_disconnect_client(&client);
	if (status == EZN_NONE) {
		EZN_INFO("Successfully disconnected client");
	} else {
		EZN_FATAL("Unable to disconnect client!");
	}

	status = ezn_safe_clean();
	if (status == EZN_ERROR) {
		EZN_WARN("Unable to successfully perform a safe clean");
	}

	ezn_clean();
	return 0;
}
