#include "coap_api.h"
#include "coap_parser.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER 1024
#define MAX_ROUTES 16

typedef struct {
    char uri[128];
    uint8_t method;
    coap_handler_fn handler;
} Route;

static Route routes[MAX_ROUTES];
static int route_count = 0;
static int running = 0;

// ===== Registro de handlers =====
int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn) {
    if (route_count >= MAX_ROUTES) return -1;
    strncpy(routes[route_count].uri, uri, sizeof(routes[route_count].uri)-1);
    routes[route_count].method = method;
    routes[route_count].handler = fn;
    route_count++;
    return 0;
}

// ===== Buscar handler =====
static coap_handler_fn find_handler(const char* uri, uint8_t method) {
    for (int i = 0; i < route_count; i++) {
        if (routes[i].method == method && strcmp(routes[i].uri, uri) == 0) {
            return routes[i].handler;
        }
    }
    return NULL;
}

int coap_server_start(int port)
{       
    
    //TODO: Implementar la logica del servidor aqui
    return 0;
}




// ===== Worker thread =====
static void* worker_thread(void* arg) {
    int sockfd = *(int*)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    uint8_t buffer[MAX_BUFFER];

    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                         (struct sockaddr*)&client_addr, &client_len);
    if (n <= 0) pthread_exit(NULL);

    coap_message_t req_msg, resp_msg;
    if (coap_parse(&req_msg, buffer, n) < 0) {
        fprintf(stderr, "Error parseando mensaje CoAP\n");
        pthread_exit(NULL);
    }

    CoapRequest req = {
        .uri = req_msg.uri_path,  // asume que coap_parse llena uri_path
        .method = req_msg.code,
        .payload = req_msg.payload,
        .payload_len = req_msg.payload_len
    };

    char resp_buf[512] = {0};
    CoapResponse resp = {
        .code = 132, // 4.04 Not Found
        .payload = resp_buf,
        .payload_size = sizeof(resp_buf)
    };

    coap_handler_fn handler = find_handler(req.uri, req.method);
    if (handler) {
        handler(&req, &resp);
    } else {
        snprintf(resp.payload, resp.payload_size, "{\"error\":\"not found\"}");
    }

    coap_build_response(&req_msg, &resp_msg, resp.code,
                        (uint8_t*)resp.payload, strlen(resp.payload));

    uint8_t out_buf[MAX_BUFFER];
    int out_len = coap_build(&resp_msg, out_buf, sizeof(out_buf));
    if (out_len > 0) {
        sendto(sockfd, out_buf, out_len, 0,
               (struct sockaddr*)&client_addr, client_len);
    }

    pthread_exit(NULL);
}

// ===== Loop principal =====
int coap_server_start(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    running = 1;
    printf("Servidor CoAP escuchando en puerto %d...\n", port);

    while (running) {
        pthread_t tid;
        if (pthread_create(&tid, NULL, worker_thread, &sockfd) == 0) {
            pthread_detach(tid);
        }
    }

    close(sockfd);
    return 0;
}

void coap_server_stop() {
    running = 0;
}
