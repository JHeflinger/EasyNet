#include "easyserver.h"

static ezn_SocketList* s_open_servers_head;

EZN_STATUS ezn_generate_server(ezn_Server* server, uint16_t port) {
	EZN_SAFECHECK();
    server->status = EZN_SERVER_CLOSED;
    if (port < MIN_PORT || port > MAX_PORT) {
        EZN_WARN("Invalid port detected. Please use a value between %d to %d", MIN_PORT, MAX_PORT);
        return EZN_ERROR;
    }
    server->port = port;
    server->type = EZN_TCP;
    return EZN_NONE;
}

EZN_STATUS ezn_open_server(ezn_Server* server) {
	EZN_SAFECHECK();
    if (server->status != EZN_SERVER_CLOSED) {
        EZN_WARN("Unable to open server that is not currently closed");
        return EZN_ERROR;
    }
    if (server->port < MIN_PORT || server->port > MAX_PORT) {
        EZN_WARN("Unable to open server with invalid port detected. Please use a value between %d to %d", MIN_PORT, MAX_PORT);
        return EZN_ERROR;
    }

    EZN_PROTOCOL protocol;
    if (server->type == EZN_TCP) protocol = EZN_TCP_PROTOCOL;
    else if (server->type == EZN_UDP) protocol = EZN_UDP_PROTOCOL;
    EZN_SOCKET server_socket = socket(AF_INET, SOCK_STREAM, protocol);
    if (server_socket == EZN_INVALID_SOCK) {
        EZN_WARN("Unable to successfully create a valid socket. Unable to open server.");
        return EZN_ERROR;
    }
	EZN_OPT_TYPE optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        EZN_WARN("Unable to set server socket options.");
        EZN_CLOSE(server_socket);
		return EZN_ERROR;
    }
    server->socket = server_socket;

    struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr.sin_port = htons((u_short)(server->port));
    if (bind(server->socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == EZN_INVALID_SOCK) {
        EZN_WARN("Unable to bind server socket");
        EZN_CLOSE(server->socket);
        return EZN_ERROR;
    }
    if (listen(server->socket, (SOMAXCONN) == EZN_INVALID_SOCK)) {
        EZN_WARN("Unable to listen for connections");
        EZN_CLOSE(server->socket);
        return EZN_ERROR;
    }
    server->status = EZN_SERVER_OPEN;
    if (s_open_servers_head == NULL) {
        s_open_servers_head = malloc(sizeof(ezn_SocketList));
        s_open_servers_head->socket = server->socket;
        s_open_servers_head->next = NULL;
    } else {
        ezn_SocketList* next_open_socket = malloc(sizeof(ezn_SocketList));
        next_open_socket->socket = server->socket;
        next_open_socket->next = s_open_servers_head;
        s_open_servers_head = next_open_socket;
    }
    return EZN_NONE;
}

EZN_STATUS ezn_close_server(ezn_Server* server) {
	EZN_SAFECHECK();
    if (server->status != EZN_SERVER_OPEN) {
        EZN_WARN("Unable to close server that is not currently open");
        return EZN_ERROR;
    }
    if (s_open_servers_head == NULL) {
        EZN_WARN("There are no servers open yet - cannot close a server");
        return EZN_ERROR;
    }
    ezn_SocketList* tracker = s_open_servers_head;
    if (tracker->socket == server->socket) {
        s_open_servers_head = tracker->next;
        free(tracker);
        EZN_CLOSE(server->socket);
    } else {
        EZN_BOOL found = EZN_FALSE;
        while (tracker->next != NULL && found == EZN_FALSE) {
            if (tracker->next->socket == server->socket) {
                ezn_SocketList* dead_node = tracker->next;
                tracker->next = tracker->next->next;
                free(dead_node);
                EZN_CLOSE(server->socket);
                found = EZN_TRUE;
            }
            tracker = tracker->next;
        }
        if (found != EZN_TRUE) {
            EZN_WARN("Unable to close an untracked server object - please use the dedicated ezn_ functions to generate and handle server objects to avoid this");
            return EZN_ERROR;
        }
    }
    server->status = EZN_SERVER_CLOSED;
    return EZN_NONE;
}

EZN_STATUS ezn_servers_are_open(EZN_BOOL* result) {
	EZN_SAFECHECK();
    if (s_open_servers_head == NULL) *result = EZN_FALSE;
    else *result = EZN_TRUE;
    return EZN_NONE;
}

EZN_STATUS ezn_clean_servers() {
	EZN_SAFECHECK();
    ezn_SocketList* tracker = s_open_servers_head;
    while (tracker != NULL) {
        EZN_CLOSE(tracker->socket);
        tracker = tracker->next;
    }
    ezn_free_socketlist(s_open_servers_head);
    s_open_servers_head = NULL;
    return EZN_NONE;
}

EZN_STATUS ezn_server_accept(ezn_Server* server, ezn_ServerBehavior behavior, EZN_BOOL implicit_close) {
	EZN_SAFECHECK();
    if (server->status != EZN_SERVER_OPEN) {
        EZN_WARN("Unable to accept connections on a server that is not open!");
        return EZN_ERROR;
    }
    int clientSocket;
    clientSocket = accept(server->socket, NULL, NULL);
    if (clientSocket == EZN_INVALID_SOCK) {
        EZN_WARN("Failed to accept a connection");
        return EZN_ERROR;
    }
    EZN_STATUS behavior_result = behavior(server, clientSocket);
    if (behavior_result == EZN_ERROR) {
        EZN_WARN("Exected behavior failed");
        EZN_CLOSE(clientSocket);
        return EZN_ERROR;
    }
    if (implicit_close == EZN_TRUE)
        EZN_CLOSE(clientSocket);
    return EZN_NONE;
}

EZN_STATUS ezn_server_queue(ezn_Server* server, ezn_ServerBehavior behavior, int queuesize, EZN_BOOL implicit_close) {
	EZN_SAFECHECK();
    while (queuesize > 0 || queuesize == EZN_ACCEPT_FOREVER) {
        if (ezn_server_accept(server, behavior, implicit_close) == EZN_ERROR) {
            EZN_WARN("An error occured while accepting a connection");
            return EZN_ERROR;
        }
        if (queuesize != EZN_ACCEPT_FOREVER) queuesize--;
    }
    return EZN_NONE;
}
