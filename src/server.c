#include "easynet.h"

#define PACKETSIZE 8
#define EXTRA_CUSTOM() printf("%s[-ME-]%s  ", EZN_PURPLE, EZN_RESET)
#define CHAT_CUSTOM() printf("%s[CHAT]%s  ", EZN_CYAN, EZN_RESET)

typedef struct {
	EZN_SOCKET client_socket;
} Handler_args;

struct PacketLog {
	char* packet;
	struct PacketLog* next;
};
typedef struct PacketLog PacketLog;

EZN_MUTEX mutex;
char input_buffer[PACKETSIZE];
int input_ind = 0;
EZN_BOOL shutdown_flag = EZN_FALSE;
PacketLog* packet_log_head = NULL;
PacketLog* packet_log_tail = NULL;
char addr[MAX_IP_ADDR_LENGTH];

void add_filled_packet() {
	PacketLog* newlog = malloc(sizeof(PacketLog));
	newlog->packet = malloc(PACKETSIZE + 1);
	memcpy(newlog->packet, input_buffer, PACKETSIZE);
	newlog->packet[PACKETSIZE] = '\0';
	newlog->next = NULL;
	if (packet_log_head == NULL) {
		packet_log_head = newlog;
		packet_log_tail = newlog;
	} else {
		packet_log_tail->next = newlog;
		packet_log_tail = newlog;
	}
	memset(input_buffer, '\0', PACKETSIZE);
	input_ind = 0;
}

void reset_prompt(EZN_BOOL logit) {
	int ind = -8;
	while ((ind < 0 || input_buffer[ind] != '\0') && ind < PACKETSIZE) {
		printf("%s", "\b \b");
		ind++;
	}
	PacketLog* tmphead = packet_log_head;
	while (tmphead != NULL) {
		for (int i = 0; i < PACKETSIZE; i++) printf("%s", "\b \b");
		tmphead = tmphead->next;
	}
	if (logit == EZN_TRUE) {
		EXTRA_CUSTOM();
		PacketLog* tmphead = packet_log_head;
		while (tmphead != NULL) {
			printf("%s", tmphead->packet);
			tmphead = tmphead->next;
		}
		printf("%s\n", input_buffer);
	}
	input_buffer[0] = '\0';
	input_ind = 0;
	PacketLog* curr = packet_log_head;
	packet_log_head = NULL;
	packet_log_tail = NULL;
	if (curr != NULL) {
		while (curr->next != NULL) {
			PacketLog* next = curr->next;
			free(curr->packet);
			free(curr);
			curr = next;
		}
		free(curr->packet);
		free(curr);
	}
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
	EZN_SOCKET clientsock = ((Handler_args*)params)->client_socket;
	size_t returnlen;
	EZN_LOCK_MUTEX(mutex);
	reset_prompt(EZN_FALSE);
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
				if (input_ind > 0 || packet_log_head != NULL) {
					input_ind--;
					if (input_ind < 0) {
						memcpy(input_buffer, packet_log_tail->packet, PACKETSIZE);
						input_buffer[PACKETSIZE - 1] = '\0';
						input_ind = PACKETSIZE - 1;
						if (packet_log_head == packet_log_tail) {
							free(packet_log_head->packet);
							free(packet_log_head);
							packet_log_head = NULL;
							packet_log_tail = NULL;
						} else {
							PacketLog* curr = packet_log_head;
							while (curr->next != packet_log_tail) {
								curr = curr->next;
							}
							free(packet_log_tail->packet);
							free(packet_log_tail);
							packet_log_tail = curr;
						}
					} else {
						input_buffer[input_ind] = '\0';
					}
					printf("%s", "\b \b");
				}
			} else if (ch == '\n' || ch == '\r') {
				if (input_buffer[0] == ';' && input_buffer[1] == ';' && input_buffer[2] == ';' && input_buffer[3] == '\0') {
					reset_prompt(EZN_FALSE);
					EZN_INFO("Killing client connection...");
					shutdown_flag = EZN_TRUE;
					EZN_RELEASE_MUTEX(mutex);
					return;
				}
				if (packet_log_head == NULL) {
					if (ezn_send(clientsock, (EZN_BYTE*)(input_buffer), strlen(input_buffer) + 1, &returnlen) == EZN_ERROR) {
						EZN_WARN("send failed");
					} else {
						char lag_buffer[PACKETSIZE];
						strcpy(lag_buffer, input_buffer);
						reset_prompt(EZN_TRUE);
						printf("\nprompt> ");
					}
				} else {
					PacketLog* curr = packet_log_head;
					while (curr != NULL) {
						if (ezn_send(clientsock, (EZN_BYTE*)(curr->packet), PACKETSIZE, &returnlen) == EZN_ERROR)
							EZN_WARN("send failed");
						curr = curr->next;
					}
					if (ezn_send(clientsock, (EZN_BYTE*)(input_buffer), strlen(input_buffer) + 1, &returnlen) == EZN_ERROR) {
						EZN_WARN("send failed");
					} else {
						char lag_buffer[PACKETSIZE];
						strcpy(lag_buffer, input_buffer);
						reset_prompt(EZN_TRUE);
						printf("\nprompt> ");
					}
				}
			} else {
				input_buffer[input_ind] = ch;
				input_ind++;
				if (input_ind >= PACKETSIZE) {
					add_filled_packet();
				} else {
					input_buffer[input_ind] = '\0';
				}
				printf("%c", ch);
			}
			EZN_RELEASE_MUTEX(mutex);
        }
    }
}

void output_handler(void* params) {
	EZN_SOCKET clientsock = ((Handler_args*)params)->client_socket;

	while (EZN_TRUE) {
		if (shutdown_flag == EZN_TRUE) return;
		PacketLog* backlog_head = NULL;
		PacketLog* backlog_tail = NULL;
		int numlogs = 0;
		size_t returnlen = 1;
		char buffer[PACKETSIZE + 1];
		memset(buffer, '\0', PACKETSIZE + 1);
		EZN_BOOL handle = EZN_FALSE;
		EZN_BOOL force_cont = EZN_FALSE;
		while (EZN_TRUE) {
			memset(buffer, '\0', PACKETSIZE + 1);
			if (ezn_ask(clientsock, (EZN_BYTE*)buffer, PACKETSIZE, &returnlen) == EZN_ERROR) {
				EZN_WARN("recieve failed");
				return;
			}
			if (returnlen > 0) {
				if (returnlen == PACKETSIZE) {
					force_cont = EZN_TRUE;
				} else force_cont = EZN_FALSE;
				handle = EZN_TRUE;
				if (buffer[PACKETSIZE - 1] != '\0') {
					numlogs++;
					PacketLog* newlog = malloc(sizeof(PacketLog));
					newlog->packet = malloc(PACKETSIZE + 1);
					memcpy(newlog->packet, buffer, returnlen);
					newlog->packet[returnlen] = '\0';
					newlog->next = NULL;
					if (backlog_head == NULL) {
						backlog_head = newlog;
						backlog_tail = newlog;
					} else {
						backlog_tail->next = newlog;
						backlog_tail = newlog;
					}
				} else {
					buffer[returnlen] = '\0';
					break;
				}
			} else {
				if (force_cont == EZN_FALSE)
					break;
			}
		}

		if (handle == EZN_TRUE) {
			if (backlog_head == NULL && buffer[0] == ';' && buffer[1] == ';' && buffer[2] == ';' && buffer[3] == '\0') {
				EZN_LOCK_MUTEX(mutex);
				reset_prompt(EZN_FALSE);
				//EZN_INFO("Client disconnected... ending chat session with client");
				printf("Client finished, now waiting to service another client...\n");
				printf("\n***********************************************************\n");
				shutdown_flag = EZN_TRUE;
				EZN_RELEASE_MUTEX(mutex);
				return;
			}
			EZN_LOCK_MUTEX(mutex);

			int num_precurses = 8 + strlen(input_buffer);
			PacketLog* curr = packet_log_head;
			while (curr != NULL) {
				num_precurses += PACKETSIZE;
				curr = curr->next;
			}

			for (int i = 0; i < num_precurses; i++) {
				printf("\b \b");
			}

			CHAT_CUSTOM();
			while (backlog_head != NULL) {
				printf("%s", backlog_head->packet);
				PacketLog* freeme = backlog_head;
				backlog_head = backlog_head->next;
				free(freeme->packet);
				free(freeme);
			}
			printf("%s\n", buffer);

			printf("\nprompt> ");
			curr = packet_log_head;
			while (curr != NULL) {
				char inter[PACKETSIZE + 1];
				memcpy(inter, curr->packet, PACKETSIZE);
				inter[PACKETSIZE] = '\0';
				printf(inter);
				curr = curr->next;
			}
			printf(input_buffer);

			EZN_RELEASE_MUTEX(mutex);
		}
	}
}

EZN_STATUS server_behavior(ezn_Server* server, EZN_SOCKET clientsock) {
	printf("\nRecieved connection request from /%s\n", addr);
	printf("\n***********************************************************\n");
	Handler_args inargs;
	Handler_args outargs;
	inargs.client_socket = clientsock;
	outargs.client_socket = clientsock;

	EZN_CREATE_MUTEX(mutex);

	shutdown_flag = EZN_FALSE;

	EZN_THREAD output_handler_thread;
	EZN_THREAD input_handler_thread;
	EZN_CREATE_THREAD(output_handler_thread, output_handler, &outargs);
	EZN_CREATE_THREAD(input_handler_thread, input_handler, &inargs);
	EZN_WAIT_THREAD(output_handler_thread);
	EZN_WAIT_THREAD(input_handler_thread);
	EZN_CLOSE_THREAD(output_handler_thread);
	EZN_CLOSE_THREAD(input_handler_thread);

	return EZN_NONE;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		EZN_FATAL("Invalid number of arguments detected - please use the program in the following format:\n\tserver <server-port>");
	}

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
}
