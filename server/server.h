#ifndef SERVER_H
#define SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "logger.h"
#include "coap_parser.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Estructura para mensajes recibidos
struct MessageData {
    uint8_t buffer[1024];
    ssize_t len;
    struct sockaddr_in client_addr;
    socklen_t client_len;
    int sockfd;
};

// Funciones del servidor
void start_server(int port, Logger *logger);
void* process_message(void* arg);
unsigned long get_thread_id(void);

#ifdef __cplusplus
}
#endif

#endif
