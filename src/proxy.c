#include "easynet.h"

#define REQUEST_SIZE 4096

EZN_MUTEX g_Mutex;
uint16_t g_Port;
ezn_Server g_Server;

void proxy(void* args) {
	EZN_SOCKET* connectionptr = (EZN_SOCKET*)args;
	EZN_SOCKET connection = *connectionptr;
	char request[REQUEST_SIZE];
	int method = 0;
	int url = 0;
	int version = 0;
	size_t returnlen;
	int status;

	memset(request, '\0', REQUEST_SIZE);
	status = ezn_recieve(connection, (EZN_BYTE*)request, REQUEST_SIZE, &returnlen);
	if (status == EZN_ERROR) {
		EZN_WARN("An error occured while recieving a client request");
	} else {
		size_t parse_ind = 0;
		int packtrack = 0;
		while (parse_ind < REQUEST_SIZE && request[parse_ind] != '\0') {
			
			prase_ind++;
		}
	}

	EZN_INFO("proxy go: %s - %s - %s", method, url, version);	

	free(connectionptr);
}

EZN_STATUS attach_client(ezn_Server* server, EZN_SOCKET clientsock) {
	EZN_SOCKET* param = calloc(1, sizeof(EZN_SOCKET));
	memcpy(param, &clientsock, sizeof(EZN_SOCKET));
	EZN_THREAD new_proxy_thread;
	EZN_CREATE_THREAD(new_proxy_thread, proxy, (void*)param);
	return EZN_NONE;
}

int main(int argc, char** argv) {
	if (argc != 2) EZN_FATAL("Invalid number of arguments! Please use the syntax \"proxy <port number>\"");
	ezn_init();

	EZN_CREATE_MUTEX(g_Mutex);

	int status;

	status = ezn_str_to_port(&g_Port, argv[1]);
	if (status == EZN_ERROR) EZN_FATAL("Invalid port given!");

	status = ezn_generate_server(&g_Server, g_Port);
	if (status == EZN_ERROR) EZN_FATAL("An error occured while generating the server!");

	status = ezn_open_server(&g_Server);
	if (status == EZN_ERROR) EZN_FATAL("An error occured while opening the server!");

	status = ezn_server_queue(&g_Server, attach_client, EZN_ACCEPT_FOREVER, EZN_FALSE);
	if (status == EZN_ERROR) EZN_WARN("An error occured while handling a new client connection - shutting down...");

	status = ezn_safe_clean();
	if (status == EZN_ERROR) {
		EZN_WARN("Unable to successfully perform a safe clean");
	}
	ezn_clean();

	return 0;
}
