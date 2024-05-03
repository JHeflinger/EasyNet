#include "easynet.h"

#define REQUEST_SIZE 4096
#define DEFAULT_HTTP_PORT 80
#define TIMEOUT 10000

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <time.h>
#else
#error "Unsupported operating system detected!"
#endif

EZN_MUTEX g_Mutex;
uint16_t g_Port;
ezn_Server g_Server;

#ifdef _WIN32
uint32_t get_unprecise_epoch() {
    FILETIME ft;
    ULONGLONG epoch_time_ms;
    GetSystemTimePreciseAsFileTime(&ft);
    epoch_time_ms = ((ULONGLONG)ft.dwHighDateTime << 32 | ft.dwLowDateTime) / 10000ULL;
    return epoch_time_ms - 0xffffffff;
}
#elif __linux__
uint32_t get_unprecise_epoch() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t epoch_time_ms = (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
	return epoch_time_ms - 0xffffffff;
}
#else
#error "Unknown operating system!"
#endif

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
					ezn_Client proxy_client = { 0 };
					proxy_client.unsafe_port_allowed = EZN_TRUE;
					if (ezn_configure_client(&proxy_client, url_port, url_ip) == EZN_ERROR) {
						error_code = 500;
					} else if (ezn_bind_client(&proxy_client) == EZN_ERROR) {
						error_code = 500;
					} else {
						size_t success_sent;
						if (ezn_send(proxy_client.socket, (EZN_BYTE*)redir_msg, strlen(redir_msg), &success_sent) == EZN_ERROR) error_code = 500;
						else if (success_sent != strlen(redir_msg)) error_code = 500;
						else {
							uint32_t rectime_start = get_unprecise_epoch();	
							while (EZN_TRUE) {
								
								if (get_unprecise_epoch() - rectime_start > TIMEOUT) {
									error_code = 505;
									break;
								}
							}
						}
						ezn_disconnect_client(&proxy_client);
					}
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
