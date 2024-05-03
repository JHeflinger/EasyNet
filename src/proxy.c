#include "easynet.h"

#define REQUEST_SIZE 4096
#define DEFAULT_HTTP_PORT 80

EZN_MUTEX g_Mutex;
uint16_t g_Port;
ezn_Server g_Server;

void proxy(void* args) {
	EZN_SOCKET* connectionptr = (EZN_SOCKET*)args;
	EZN_SOCKET connection = *connectionptr;
	char request[REQUEST_SIZE];
	int request_coordinates[3]; // 0 is the method, 1 is the url, and 2 is the version
	#define PROXY_METHOD (request + request_coordinates[0])
	#define PROXY_URL (request + request_coordinates[1])
	#define PROXY_VERSION (request + request_coordinates[2])
	size_t returnlen;
	int status;

	int error_code = 0;
	memset(request, '\0', REQUEST_SIZE);
	memset(request_coordinates, 0, sizeof(int)*3);
	status = ezn_recieve(connection, (EZN_BYTE*)request, REQUEST_SIZE, &returnlen);
	if (status == EZN_ERROR) {
		EZN_WARN("An error occured while recieving a client request");
	} else {
		size_t parse_ind = 0;
		int packtrack = 0;
		while (parse_ind < REQUEST_SIZE && request[parse_ind] != '\0') {
			if (request[parse_ind] == '\n' && request[parse_ind + 1] == '\0')
				request[parse_ind] = '\0';
			parse_ind++;
		}
		parse_ind = 0;
		while (parse_ind < REQUEST_SIZE && request[parse_ind] != '\0') {
			if (request[parse_ind] == ' ') {
				packtrack++;
				request[parse_ind] = '\0';
				request_coordinates[packtrack] = parse_ind + 1;
			}
			parse_ind++;
		}
		if (packtrack != 2) error_code = 400;
		if (error_code == 0) {
			if (strcmp(PROXY_METHOD, "GET") == 0) {
				uint16_t url_port;
				uint8_t url_ip[4];
				char full_url[REQUEST_SIZE];
				char path[REQUEST_SIZE];
				char hostname[REQUEST_SIZE];
				strcpy(full_url, PROXY_URL);
				parse_ind = 0;
				int num_slashes = 0;
				int window_start = 0;
				int port_ind = -1;
				while (full_url[parse_ind] != '\0') {
					if (full_url[parse_ind] == '/')  {
						num_slashes++;
					} else {
						if (num_slashes == 2) {
							window_start = parse_ind;
							while(full_url[parse_ind] != '/' && full_url[parse_ind] != '\0') {
								if (full_url[parse_ind] == '\0') error_code = 400;
								else if (full_url[parse_ind] == ':') port_ind = parse_ind + 1;
								parse_ind++;
							}
							full_url[parse_ind] = '\0';
							if (port_ind == -1) { 
								url_port = DEFAULT_HTTP_PORT; 
								strcpy(hostname, full_url + window_start);
							} else if (ezn_str_to_port(&url_port, full_url + port_ind) == EZN_ERROR) {
								error_code = 400;
							} else {
								full_url[port_ind] = '\0';
								strcpy(hostname, full_url + window_start);
							}
							window_start = parse_ind + 1;
							break;
						}
					}
					parse_ind++;
				}
				strcpy(path, full_url + window_start);
				if (ezn_hostname_to_ip(hostname, url_ip) == EZN_ERROR) error_code = 404;
				if (error_code == 0) {
					char redir_msg[REQUEST_SIZE*3];
					sprintf(redir_msg, "GET /%s %s\nHost: %s\nConnection: close\n", path, PROXY_VERSION, hostname);
					printf("%s", redir_msg);
				}
				EZN_INFO("error code: %d", error_code);
				EZN_INFO("address ip: %d.%d.%d.%d", url_ip[0], url_ip[1], url_ip[2], url_ip[3]);	
				EZN_INFO("url info: %s : %d / %s", hostname, (int)url_port, path);	
			} else {
				error_code = 501;
			}
		}
	}

	EZN_INFO("proxy go: %s - %s - %s", PROXY_METHOD, PROXY_URL, PROXY_VERSION);	

	free(connectionptr);
	#undef PROXY_METHOD 
	#undef PROXY_URL 
	#undef PROXY_VERSION
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
