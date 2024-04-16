#include "easynet.h"

#ifdef _WIN32
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PACKETSIZE 4096
#define HEADERSIZE 8

typedef enum {
	DEFAULT,
	IWANTASKDIR,
	UTAKEASKDIR,
} FT_State;

typedef struct {
	EZN_SOCKET socket_arg;
} Handler_args;

EZN_MUTEX mutex;
char output_buffer[PACKETSIZE];
char input_buffer[PACKETSIZE];
EZN_BOOL shutdown_flag = EZN_FALSE;
EZN_BOOL lock_rec = EZN_FALSE;
char addr[MAX_IP_ADDR_LENGTH];
FT_State ftstate = DEFAULT;

void input_handler(void* params) {
	EZN_SOCKET connectedsocket = ((Handler_args*)params)->socket_arg;
	size_t returnlen;
	EZN_LOCK_MUTEX(mutex);
	printf("> ");
	EZN_RELEASE_MUTEX(mutex);
	char file_to_handle[PACKETSIZE];
	while (EZN_TRUE) {
		if (shutdown_flag == EZN_TRUE) return;
		scanf("%[^\n]%*c", input_buffer);
		EZN_LOCK_MUTEX(mutex);
		char process_cmd[PACKETSIZE*2];
		char out_cmd[PACKETSIZE];
		memcpy(process_cmd, input_buffer, PACKETSIZE);
		process_cmd[5] = '\0';
		if (strcmp(input_buffer, "exit") == 0) {
			printf("Say byebye~ to the client!...\n");
			shutdown_flag = EZN_TRUE;
			EZN_RELEASE_MUTEX(mutex);
			return;
		} else if (ftstate == IWANTASKDIR) {
			#ifdef _WIN32
			struct _stat64i32 info;
			EZN_BOOL is_real = _stat(input_buffer, &info) == 0;
			#else
			struct stat info;
			EZN_BOOL is_real = stat(input_buffer, &info) == 0;
			#endif
			printf("> ");
			if (is_real) {
				lock_rec = EZN_TRUE;
				if (ezn_send(connectedsocket, (EZN_BYTE*)file_to_handle, strlen(file_to_handle) + 1, &returnlen) == EZN_ERROR) {
					EZN_WARN("request for file failed to send");
				} else {
					char header[HEADERSIZE];
					if (ezn_recieve(connectedsocket, (EZN_BYTE*)header, HEADERSIZE, &returnlen) == EZN_ERROR) {
						EZN_WARN("unable to recieve header");
					} else {
						if (returnlen > 0 && returnlen < PACKETSIZE) {
							uint64_t numbytes;
							memcpy(&numbytes, header, sizeof(uint64_t));
							EZN_INFO("TODO: btw ready to recieve %d bytes", (int)numbytes);
						} else {
							EZN_WARN("recieved incomplete header");
						}
					}
				}
				lock_rec = EZN_FALSE;
			} else {
				printf("That location doesn't exist dummy!\n");
			}
			ftstate = DEFAULT;
		} else if (ftstate == UTAKEASKDIR) {
					
		} else if (strcmp(process_cmd, "iWant") == 0 && ftstate == DEFAULT) {
			lock_rec = EZN_TRUE;
			memcpy(process_cmd + 2, input_buffer + 6, PACKETSIZE - 6);
			process_cmd[strlen(input_buffer) + 2] = '\0';
			process_cmd[0] = 'a';
			process_cmd[1] = 'e';
			if (ezn_send(connectedsocket, (EZN_BYTE*)process_cmd, strlen(process_cmd) + 1, &returnlen) == EZN_ERROR) {
				EZN_WARN("send failed");
			} else {
				if (ezn_recieve(connectedsocket, (EZN_BYTE*)out_cmd, 3, &returnlen) == EZN_ERROR) {
					EZN_WARN("receive failed");
				} else {
					if (out_cmd[0] == 'c' && out_cmd[1] == 'e') {
						if (out_cmd[2] == 'y') {
							strcpy(file_to_handle + 1, process_cmd + 2);
							file_to_handle[0] = 'd';
							printf("  What directory would you like to save this file?\n");
							ftstate = IWANTASKDIR;
						} else if (out_cmd[2] == 'n') {
							printf("  What you talkin' bout Willis? I ain't seen that file anywhere!\n");
						} else {
							EZN_WARN("Invalid response");
						}
					} else {
						EZN_WARN("Invalid response");
					}
				}
			}
			lock_rec = EZN_FALSE;
			printf("> ");
		} else if (strcmp(process_cmd, "uTake") == 0 && ftstate == DEFAULT) {
			printf("  you did an utake command!\n");
			printf("> ");
		} else {
			printf("  That just ain't right!\n");
			printf("> ");
		}
		EZN_RELEASE_MUTEX(mutex);
    }
}

void net_handler(void* params) {
	EZN_SOCKET connectedsocket = ((Handler_args*)params)->socket_arg;
	char netbuffer[PACKETSIZE*2];
	memset(netbuffer, '\0', PACKETSIZE*2);
	size_t retlen;
	while (EZN_TRUE) {
		if (shutdown_flag == EZN_TRUE) return;
		if (lock_rec == EZN_FALSE) {
			if (ezn_ask(connectedsocket, (EZN_BYTE*)netbuffer, PACKETSIZE*2, &retlen) == EZN_ERROR) {
				EZN_WARN("Error occured while asking for data");
			} else {
				if (retlen > 0 && retlen <= PACKETSIZE*2) {
					#ifdef _WIN32
					struct _stat64i32 info;
					EZN_BOOL is_real = _stat(netbuffer + 2, &info) == 0;
					#else
					struct stat info;
					EZN_BOOL is_real = stat(netbuffer + 2, &info) == 0;
					#endif
					netbuffer[0] = 'c';
					netbuffer[1] = 'e';
					if (is_real) {
						netbuffer[2] = 'y';
						if (ezn_send(connectedsocket, (EZN_BYTE*)netbuffer, 3, &retlen) == EZN_ERROR) EZN_WARN("Error occured while sending");
					} else {
						netbuffer[2] = 'n';
						if (ezn_send(connectedsocket, (EZN_BYTE*)netbuffer, 3, &retlen) == EZN_ERROR) EZN_WARN("Error occured while sending");
					}
				}
				memset(netbuffer, '\0', PACKETSIZE*2);
			}
		}
	}
}

EZN_STATUS my_behavior(EZN_SOCKET connectedsocket) {
	printf("Connection established, now waiting for user input...\n");

	Handler_args inargs;
	Handler_args netargs;
	inargs.socket_arg = connectedsocket;
	netargs.socket_arg = connectedsocket;

	EZN_CREATE_MUTEX(mutex);

	EZN_THREAD input_handler_thread;
	EZN_THREAD net_handler_thread;
	EZN_CREATE_THREAD(input_handler_thread, input_handler, &inargs);
	EZN_CREATE_THREAD(net_handler_thread, net_handler, &netargs);
	EZN_WAIT_THREAD(input_handler_thread);
	EZN_WAIT_THREAD(net_handler_thread);
	EZN_CLOSE_THREAD(input_handler_thread);
	EZN_CLOSE_THREAD(net_handler_thread);

	return EZN_NONE;
}

EZN_STATUS server_behavior(ezn_Server* server, EZN_SOCKET connectedsocket) {
	return my_behavior(connectedsocket);
}

EZN_STATUS client_behavior(ezn_Client* client, EZN_SOCKET connectedsocket) {
	return my_behavior(connectedsocket);
}

int main(int argc, char* argv[]) {
	if (argc == 2) {
		ezn_init();

		uint16_t port;

		if (ezn_str_to_port(&port, argv[1]) == EZN_ERROR) {
			EZN_FATAL("Invalid port detected, please use a valid port from %d to %d.", MIN_PORT, MAX_PORT);
		}

		EZN_STATUS status = ezn_hostaddress(addr, MAX_HOST_NAME_LENGTH);
		if (status == EZN_NONE) {
			//EZN_INFO("The host ip is %s", addr);
		} else {
			EZN_FATAL("Host IP could not be detected, internal error found");
		}

		ezn_Server server;
		status = ezn_generate_server(&server, port);
		if (status == EZN_NONE) {
			//EZN_INFO("Successfully generated server device for port %d", server.port);
			printf("\nSerial Server on host 0.0.0.0/0.0.0.0 is listening on port %s\n", argv[1]);
		} else {
			EZN_FATAL("Unable to generate server device");
		}
		
		status = ezn_open_server(&server);
		if (status == EZN_NONE) {
			//EZN_INFO("Successfully opened server on port %d", server.port);
			printf("\nSerial Server starting, listening on port %s\n", argv[1]);
		} else {
			EZN_FATAL("Unable to open server");
		}

		status = ezn_server_queue(&server, server_behavior, EZN_ACCEPT_FOREVER, EZN_TRUE);
		if (status == EZN_NONE) {
			EZN_INFO("Finished taking clients");
		} else {
			EZN_WARN("Unable to properly execute server behavior");
		}

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
	} else if (argc == 3) {
		ezn_init();

		ezn_Client client;
		EZN_STATUS status;
		uint8_t address[IPV4_ADDR_LENGTH];
		uint16_t port;

		if (ezn_str_to_ipaddr(address, argv[1]) == EZN_ERROR) {
			EZN_FATAL("Invalid IP address detected, please use a valid IPv4 address.");
		}

		if (ezn_str_to_port(&port, argv[2]) == EZN_ERROR) {
			EZN_FATAL("Invalid port detected, please use a valid port from %d to %d.", MIN_PORT, MAX_PORT);
		}
		
		char addrstr[1024];
		ezn_ipaddr_to_str(address, addrstr, 1024);
		//EZN_INFO("Connecting client to %s on port %d", addrstr, port);

		status = ezn_configure_client(&client, port, address);
		if (status == EZN_NONE) {
			//EZN_INFO("Successfully configured client");
			printf("\nClient has requested to start connection with host %s on port %s\n", argv[1], argv[2]);
			printf("\n***********************************************************\n\n");
		} else {
			EZN_FATAL("An error occurred while configuring the client");
		}

		status = ezn_connect_client(&client, client_behavior);
		if (status == EZN_NONE) {
			//EZN_INFO("Finished talking to server!");
			printf("Attempting to shut down client sockets and other streams\n\n");
		} else {
			EZN_WARN("Unable to properly connect client");
		}

		status = ezn_disconnect_client(&client);
		if (status == EZN_NONE) {
			//EZN_INFO("Successfully disconnected client");
		} else {
			EZN_FATAL("Unable to disconnect client!");
		}

		status = ezn_safe_clean();
		if (status == EZN_ERROR) {
			EZN_WARN("Unable to successfully perform a safe clean");
		} else {
			printf("Shut down successful.... goodbye\n");
		}

		ezn_clean();
		return 0;
	} else {
		EZN_FATAL("Invalid number of arguments detected");
	}
}
