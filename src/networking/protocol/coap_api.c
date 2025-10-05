#include "coap_api.h"
#include "coap_parser.h"
#include "coap_router.h"
#include "server.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER 1024
#define MAX_ROUTES 16

static Route routes[MAX_ROUTES];
static int route_count = 0;

int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn) {
    if (route_count >= MAX_ROUTES) return -1;
    strncpy(routes[route_count].uri, uri, sizeof(routes[route_count].uri)-1);
    routes[route_count].method = method;
    routes[route_count].handler = fn;
    route_count++;
    return 0;
}

coap_handler_fn find_handler(const char* uri, uint8_t method) {
    for (int i = 0; i < route_count; i++) {
        if (routes[i].method == method && strcmp(routes[i].uri, uri) == 0) {
            return routes[i].handler;
        }
    }
    return NULL;
}

int coap_server_start(int port,const char *logFileName)
{           
    Logger *logger = logger_init(logFileName);
    if (!logger) {
        fprintf(stderr, "Error al inicializar logger\n");
        return 1;
    }

    logger_log(logger, "Servidor iniciado...");

    start_server(port, logger);

    logger_cleanup(logger);

    return 0;
}


