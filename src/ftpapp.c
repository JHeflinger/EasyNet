#include "easynet.h"

#ifdef _WIN32
#include <sys/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define PACKETSIZE 4096

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
int input_ind = 0;
EZN_BOOL shutdown_flag = EZN_FALSE;
char addr[MAX_IP_ADDR_LENGTH];
FT_State ftstate = DEFAULT;

void strapp(char* str, char* nextstr) {
	strcpy(str + strlen(str), nextstr);
}

void reset_prompt() {
	int ind = -8;
	while ((ind < 0 || input_buffer[ind] != '\0') && ind < PACKETSIZE) {
		printf("%s", "\b \b");
		ind++;
	}
	input_buffer[0] = '\0';
	input_ind = 0;
}

#ifdef __linux__
int _kbhit(void) {
	struct termios oldt, newt;
	int ch;
	int oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
 	return 0;
}
#endif

void input_handler(void* params) {
	EZN_SOCKET connectedsocket = ((Handler_args*)params)->socket_arg;
	size_t returnlen;
	EZN_LOCK_MUTEX(mutex);
	reset_prompt();
	printf("\nprompt> ");
	EZN_RELEASE_MUTEX(mutex);
	while (EZN_TRUE) {
		if (shutdown_flag == EZN_TRUE) return;
		if (_kbhit()) {
		#ifdef __linux__
            char ch = getchar();
		#elif _WIN32
			char ch = _getch();
		#endif
			EZN_LOCK_MUTEX(mutex);
			if (ch == '\b' || ch == 127) {
				if (input_ind > 0) {
					input_ind--;
                    input_buffer[input_ind] = '\0';
					printf("%s", "\b \b");
				}
			} else if (ch == '\n' || ch == '\r') {
				char process_cmd[PACKETSIZE*2];
				char out_cmd[PACKETSIZE];
				memcpy(process_cmd, input_buffer, PACKETSIZE);
				process_cmd[5] = '\0';
				if (strcmp(input_buffer, "exit") == 0) {
					reset_prompt();
					printf("Say byebye~ to the client!...\n");
					shutdown_flag = EZN_TRUE;
					EZN_RELEASE_MUTEX(mutex);
					return;
				} else if (ftstate == IWANTASKDIR) {

				} else if (ftstate == UTAKEASKDIR) {
					
				} else if (strcmp(process_cmd, "iWant") == 0 && ftstate == DEFAULT) {
					memcpy(process_cmd + 2, input_buffer + 6, PACKETSIZE - 6);
					process_cmd[input_ind + 2] = '\0';
					process_cmd[0] = 'a';
					process_cmd[1] = 'e';
					strcpy(output_buffer, "> ");
					strapp(output_buffer, input_buffer);
					if (ezn_send(connectedsocket, (EZN_BYTE*)(process_cmd), strlen(process_cmd) + 1, &returnlen) == EZN_ERROR) {
						EZN_WARN("send failed");
					} else {
						if (ezn_recieve(connectedsocket, out_cmd, 3, &returnlen) == EZN_ERROR) {
							EZN_WARN("receive failed");
						} else {
							if (out_cmd[0] == 'c' && out_cmd[1] == 'e') {
								if (out_cmd[2] == 'y') {
									strapp(output_buffer, "\n  What directory would you like to save this file?");
									ftstate = IWANTASKDIR;
								} else if (out_cmd[2] == 'n') {
									strapp(output_buffer, "\n  What you talkin' bout Willis? I ain't seen that file anywhere!");
								} else {
									EZN_WARN("Invalid response");
								}
							} else {
								EZN_WARN("Invalid response");
							}
						}
					}
					reset_prompt();
					printf("prompt> ");
				} else if (strcmp(process_cmd, "uTake") == 0 && ftstate == DEFAULT) {
					strapp(output_buffer, "\n  you did an utake command!");
					reset_prompt();
					printf("prompt> ");
				} else {
					strapp(output_buffer, "\n  That just ain't right!");
					reset_prompt();
					printf("prompt> ");
				}
			} else {
				input_buffer[input_ind] = ch;
				input_ind++;
                input_buffer[input_ind] = '\0';
				printf("%c", ch);
			}
			EZN_RELEASE_MUTEX(mutex);
        }
    }
}

void output_handler(void* params) {
	EZN_SOCKET connectedsocket = ((Handler_args*)params)->socket_arg;
    memset(output_buffer, '\0', PACKETSIZE);

	while (EZN_TRUE) {
		if (shutdown_flag == EZN_TRUE) return;
        EZN_LOCK_MUTEX(mutex);
        if (output_buffer[0] != '\0') {
	        int ind = -8;
	        while ((ind < 0 || input_buffer[ind] != '\0') && ind < PACKETSIZE) {
		        printf("%s", "\b \b");
		        ind++;
	        }
            printf("%s\n", output_buffer);
            printf("prompt> %s", input_buffer);
			memset(output_buffer, '\0', PACKETSIZE);
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
		if (ezn_ask(connectedsocket, netbuffer, PACKETSIZE*2, &retlen) == EZN_ERROR) {
			EZN_WARN("Error occured while asking for data");
		} else {
			if (retlen > 0 && retlen <= PACKETSIZE*2) {
				#ifdef _WIN32
				struct _stat64i32 info;
				EZN_BOOL is_directory = _stat(netbuffer + 2, &info) == 0;
				#else
				struct stat info;
				EZN_BOOL is_directory = stat(netbuffer + 2, &info) == 0;
				#endif
				netbuffer[0] = 'c';
				netbuffer[1] = 'e';
				if (is_directory) {
					netbuffer[2] = 'y';
					if (ezn_send(connectedsocket, netbuffer, 3, &retlen) == EZN_ERROR) EZN_WARN("Error occured while sending");
				} else {
					netbuffer[2] = 'n';
					if (ezn_send(connectedsocket, netbuffer, 3, &retlen) == EZN_ERROR) EZN_WARN("Error occured while sending");
				}
			}
			memset(netbuffer, '\0', PACKETSIZE*2);
		}
	}
}

EZN_STATUS my_behavior(ezn_Client* client, EZN_SOCKET connectedsocket) {
	printf("Connection established, now waiting for user input...\n");

	Handler_args inargs;
	Handler_args outargs;
	Handler_args netargs;
	inargs.socket_arg = connectedsocket;
	outargs.socket_arg = connectedsocket;
	netargs.socket_arg = connectedsocket;

	EZN_CREATE_MUTEX(mutex);

	EZN_THREAD output_handler_thread;
	EZN_THREAD input_handler_thread;
	EZN_THREAD net_handler_thread;
	EZN_CREATE_THREAD(output_handler_thread, output_handler, &outargs);
	EZN_CREATE_THREAD(input_handler_thread, input_handler, &inargs);
	EZN_CREATE_THREAD(net_handler_thread, net_handler, &netargs);
	EZN_WAIT_THREAD(output_handler_thread);
	EZN_WAIT_THREAD(input_handler_thread);
	EZN_WAIT_THREAD(net_handler_thread);
	EZN_CLOSE_THREAD(output_handler_thread);
	EZN_CLOSE_THREAD(input_handler_thread);
	EZN_CLOSE_THREAD(net_handler_thread);

	return EZN_NONE;
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

		status = ezn_server_queue(&server, my_behavior, EZN_ACCEPT_FOREVER, EZN_TRUE);
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

		status = ezn_connect_client(&client, my_behavior);
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
