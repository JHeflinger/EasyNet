#pragma once
#include "netbasic.h"
#include "easystructs.h"
#include <stdint.h>

#define EZN_TCP 0
#define EZN_UDP 1
typedef int EZN_SERVER_TYPE;

#define EZN_SERVER_OPEN 0
#define EZN_SERVER_CLOSED 1
typedef int EZN_SERVER_STATUS;

typedef struct {
    uint16_t port;
    uint32_t socket;
    EZN_SERVER_STATUS status;
    EZN_SERVER_TYPE type;
} ezn_Server;

typedef EZN_STATUS (*ezn_Behavior)(ezn_Server*, uint32_t);

static ezn_SocketList* s_open_servers_head;

EZN_STATUS ezn_generate_server(ezn_Server* server, uint16_t port);
EZN_STATUS ezn_open_server(ezn_Server* server);
EZN_STATUS ezn_close_server(ezn_Server* server);
EZN_STATUS ezn_servers_are_open(EZN_BOOL* result);
EZN_STATUS ezn_clean_servers();
EZN_STATUS ezn_server_accept(ezn_Server* server, ezn_Behavior behavior);

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

#ifdef __linux__

EZN_STATUS ezn_open_server(ezn_Server* server) {
    #error "Not implemented!"
	EZN_SAFECHECK();
    return EZN_NONE;
}

EZN_STATUS ezn_close_server(ezn_Server* server) {
    #error "Not implemented!"
	EZN_SAFECHECK();
    return EZN_NONE;
}

EZN_STATUS ezn_servers_are_open(EZN_BOOL* result) {
    #error "Not implemented!"
	EZN_SAFECHECK();
    return EZN_NONE;
}

EZN_STATUS ezn_clean_servers() {
    #error "Not implemented!"
	EZN_SAFECHECK();
    return EZN_NONE;
}

EZN_STATUS ezn_server_accept(ezn_Server* server, ezn_Behavior behavior) {
    #error "Not implemented!"
	EZN_SAFECHECK();
    return EZN_NONE;
}

#elif _WIN32

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

    IPPROTO protocol;
    if (server->type == EZN_TCP) protocol = IPPROTO_TCP;
    else if (server->type == EZN_UDP) protocol = IPPROTO_UDP;
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, protocol);
    if (server_socket == INVALID_SOCKET) {
        EZN_WARN("Unable to successfully create a valid socket. Unable top open server.");
        return EZN_ERROR;
    }
    server->socket = (uint32_t)server_socket;

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces
    serverAddr.sin_port = htons((u_short)(server->port));
    if (bind((SOCKET)(server->socket), (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        EZN_WARN("Unable to bind server socket");
        closesocket((SOCKET)(server->socket));
        return EZN_ERROR;
    }
    if (listen((SOCKET)(server->socket), (SOMAXCONN) == SOCKET_ERROR)) {
        EZN_WARN("Unable to listen for connections");
        closesocket((SOCKET)(server->socket));
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
        closesocket((SOCKET)(server->socket));
    } else {
        EZN_BOOL found = EZN_FALSE;
        while (tracker->next != NULL && found == EZN_FALSE) {
            if (tracker->next->socket = server->socket) {
                ezn_SocketList* dead_node = tracker->next;
                tracker->next = tracker->next->next;
                free(dead_node);
                closesocket((SOCKET)(server->socket));
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
        closesocket((SOCKET)(tracker->socket));
        tracker = tracker->next;
    }
    ezn_free_socketlist(s_open_servers_head);
    s_open_servers_head = NULL;
    return EZN_NONE;
}

EZN_STATUS ezn_server_accept(ezn_Server* server, ezn_Behavior behavior) {
	EZN_SAFECHECK();
    if (server->status != EZN_SERVER_OPEN) {
        EZN_WARN("Unable to accept connections on a server that is not open!");
        return EZN_ERROR;
    }
    SOCKET clientSocket;
    clientSocket = accept(server->socket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        EZN_WARN("Failed to accept a connection");
        return EZN_ERROR;
    }
    EZN_STATUS behavior_result = behavior(server, (uint32_t)clientSocket);
    if (behavior_result == EZN_ERROR) {
        EZN_WARN("Exected behavior failed");
        closesocket(clientSocket);
        return EZN_ERROR;
    }
    closesocket(clientSocket);
    return EZN_NONE;
}

#else
#error Unsupported operating system detected!
#endif