#include "easyclient.h"

static ezn_SocketList* s_open_clients_head;

EZN_STATUS ezn_configure_client(ezn_Client* client, uint16_t port, uint8_t* address) {
	EZN_SAFECHECK();
	client->status = EZN_CLIENT_DISCONNECTED;
	if (port < MIN_PORT || port > MAX_PORT) {
		EZN_WARN("Invalid port detected. Please use a value between %d to %d", MIN_PORT, MAX_PORT);
		return EZN_ERROR;
	}
	client->port = port;
	client->type = EZN_TCP;
	memcpy(client->address, address, IPV4_ADDR_LENGTH);
	return EZN_NONE;
}

EZN_STATUS ezn_connect_client(ezn_Client* client, ezn_ClientBehavior behavior) {
	EZN_SAFECHECK();
	if (client->status != EZN_CLIENT_DISCONNECTED) {
		EZN_WARN("Unable to connect a client that is already connected");
		return EZN_ERROR;
	}
	if (client->port < MIN_PORT || client->port > MAX_PORT) {
		EZN_WARN("Unable to connect client with an invalid port. Please use a value between %d to %d", MIN_PORT, MAX_PORT);
		return EZN_ERROR;
	}
	
	EZN_PROTOCOL protocol;
	if (client->type == EZN_TCP) protocol = EZN_TCP_PROTOCOL;
	else if (client->type == EZN_UDP) protocol = EZN_UDP_PROTOCOL;
	EZN_SOCKET client_socket = socket(AF_INET, SOCK_STREAM, protocol);
	if (client_socket == EZN_INVALID_SOCK) {
		EZN_WARN("Unable to successfully create a valid socket. Unable to connect the client");
		return EZN_ERROR;
	}
	client->socket = client_socket;

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((u_short)(client->port));
	char addrstr[1024];
	ezn_ipaddr_to_str(client->address, addrstr, 1024);
	if (inet_pton(AF_INET, addrstr, &serverAddr.sin_addr) <= 0) {
		EZN_WARN("Client has invalid IP address, unable to connect");
		EZN_CLOSE(client->socket);
		return EZN_ERROR;
	}

	if (connect(client->socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		EZN_WARN("Unable to connect configured client to server");
		EZN_CLOSE(client->socket);
		return EZN_ERROR;
	}

	client->status = EZN_CLIENT_CONNECTED;
	if (s_open_clients_head == NULL) {
		s_open_clients_head = malloc(sizeof(ezn_SocketList));
		s_open_clients_head->socket = client->socket;
		s_open_clients_head->next = NULL;
	} else {
		ezn_SocketList* next_open_socket = malloc(sizeof(ezn_SocketList));
		next_open_socket->socket = client->socket;
		next_open_socket->next = s_open_clients_head;
		s_open_clients_head = next_open_socket;
	}

	EZN_STATUS behavior_result = behavior(client, client->socket);
	if (behavior_result == EZN_ERROR) {
		EZN_WARN("An error occurred while performing the given dynamic behavior");
		EZN_CLOSE(client->socket);
		return EZN_ERROR;
	}

	return EZN_NONE;
}

EZN_STATUS ezn_disconnect_client(ezn_Client* client) {
	EZN_SAFECHECK();
	if (client->status != EZN_CLIENT_CONNECTED) {
		EZN_WARN("Unable to disconnect a client that is not already connected");
		return EZN_ERROR;
	}
	if (s_open_clients_head == NULL) {
		EZN_WARN("There are no clients connected yet - cannot close a client");
		return EZN_ERROR;
	}
	ezn_SocketList* tracker = s_open_clients_head;
	if (tracker->socket == client->socket) {
		s_open_clients_head = tracker->next;
		free(tracker);
		EZN_CLOSE(client->socket);
	} else {
		EZN_BOOL found = EZN_FALSE;
		while (tracker->next != NULL && found == EZN_FALSE) {
			if (tracker->next->socket == client->socket) {
				ezn_SocketList* dead_node = tracker->next;
				tracker->next = tracker->next->next;
				free(dead_node);
				EZN_CLOSE(client->socket);
				found = EZN_TRUE;
			}
			tracker = tracker->next;
		}
		if (found != EZN_TRUE) {
			EZN_WARN("Unable to close an untracked server object - please use the dedicated ezn_ functions to generate and handle client objects to avoid this");
			return EZN_ERROR;
		}
	}
	client->status = EZN_CLIENT_DISCONNECTED;
	return EZN_NONE;
}

EZN_STATUS ezn_clients_are_open(EZN_BOOL* result) {
	EZN_SAFECHECK();
	if (s_open_clients_head == NULL) *result = EZN_FALSE;
	else *result = EZN_TRUE;
	return EZN_NONE;
}

EZN_STATUS ezn_clean_clients() {
	EZN_SAFECHECK();
	ezn_SocketList* tracker = s_open_clients_head;
	while (tracker != NULL) {
		EZN_CLOSE(tracker->socket);
		tracker = tracker->next;
	}
	ezn_free_socketlist(s_open_clients_head);
	s_open_clients_head = NULL;
	return EZN_NONE;
}

