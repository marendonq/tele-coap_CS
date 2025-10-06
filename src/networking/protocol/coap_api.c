#include "coap_api.h"
#include "coap_parser.h"
#include "coap_router.h"
#include "message.h"
#include "server.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>


#define MAX_BUFFER 1024
#define MAX_ROUTES 16
#define MAX_TRANSACTIONS 32

static CoapTransaction transactions[MAX_TRANSACTIONS];
static Route routes[MAX_ROUTES];
static int route_count = 0;
static int running = 0;

static uint16_t next_message_id() {
    static uint16_t mid = 1;
    return mid++;
}

static void generate_token(uint8_t *token, uint8_t *len) {
    *len = 2; // RFC: mínimo 1 byte, máximo 8
    token[0] = rand() & 0xFF;
    token[1] = rand() & 0xFF;
}

static void register_transaction(uint16_t mid, const uint8_t *token, uint8_t tkl) {
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (!transactions[i].active) {
            transactions[i].message_id = mid;
            memcpy(transactions[i].token, token, tkl);
            transactions[i].token_len = tkl;
            transactions[i].active = 1;
            return;
        }
    }
}

static void clear_transaction(uint16_t mid) {
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (transactions[i].active && transactions[i].message_id == mid) {
            transactions[i].active = 0;
            return;
        }
    }
}

int coap_send_request(const char *host, int port, const char *path,const char *method_str, const char *mode_str,const char *json_payload) {
    int method_code = COAP_METHOD_GET;
    if (strcmp(method_str, "get") == 0) method_code = COAP_METHOD_GET;
    else if (strcmp(method_str, "post") == 0) method_code = COAP_METHOD_POST;
    else if (strcmp(method_str, "put") == 0) method_code = COAP_METHOD_PUT;
    else if (strcmp(method_str, "delete") == 0) method_code = COAP_METHOD_DELETE;

    int confirmable = (strcmp(mode_str, "con") == 0) ? 1 : 0;

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr; 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "IP inválida: %s\n", host);
        close(sockfd);
        return 1;
    }

    unsigned char buffer[1024];
    CoapMessage msg; coap_message_init(&msg);

    msg.type = confirmable ? 0 : 1; // CON = 0, NON = 1
    msg.code = (unsigned char)method_code;
    msg.message_id = next_message_id();

    msg.tkl = 0;
    generate_token(msg.token, &msg.tkl);

    // Guardar transacción
    register_transaction(msg.message_id, msg.token, msg.tkl);

    // Dividir el path en opciones Uri-Path
    const char *p = path;
    while (*p == '/') p++;
    while (*p) {
        const char *start = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - start);
        if (len) coap_add_option(&msg, 11, (const unsigned char*)start, (unsigned short)len);
        while (*p == '/') p++;
    }

    // Payload para POST/PUT
    if ((method_code == COAP_METHOD_POST || method_code == COAP_METHOD_PUT) &&
        json_payload && strlen(json_payload) > 0) {
        unsigned char cf = 50; // application/json
        coap_add_option(&msg, 12, &cf, 1);
        coap_set_payload(&msg, (const unsigned char*)json_payload, (int)strlen(json_payload));
    }

    int len = coap_serialize(&msg, buffer, sizeof(buffer));
    if (len <= 0) {
        fprintf(stderr, "Error al serializar CoAP\n");
        close(sockfd);
        return 1;
    }

    ssize_t sent = sendto(sockfd, buffer, (size_t)len, 0,
                          (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
        perror("sendto");
        close(sockfd);
        return 1;
    }
    printf("-> %s %s (MID=%u, %zd bytes)\n", method_str, path, msg.message_id, sent);

    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    unsigned char recv_buf[1024]; socklen_t addr_len = sizeof(server_addr);
    ssize_t recvd = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0,
                             (struct sockaddr*)&server_addr, &addr_len);
    if (recvd > 0) {
        CoapMessage resp;
        if (coap_parse(&resp, recv_buf, (int)recvd) == 0) {
            // Imprimir información detallada de la respuesta
            printf("<- Respuesta recibida:\n");
            printf("   Código: %d", resp.code);
            
            // Interpretar código de respuesta
            switch(resp.code) {
                case 65: printf(" (2.01 Created)"); break;
                case 66: printf(" (2.02 Deleted)"); break;
                case 67: printf(" (2.03 Valid)"); break;
                case 68: printf(" (2.04 Changed)"); break;
                case 69: printf(" (2.05 Content)"); break;
                case 132: printf(" (4.04 Not Found)"); break;
                case 160: printf(" (5.00 Internal Server Error)"); break;
                case 199: printf(" (NON response)"); break;
                default: printf(" (Código desconocido)"); break;
            }
            printf("\n");
            
            printf("   Tipo: %d", resp.type);
            switch(resp.type) {
                case 0: printf(" (CON - Confirmable)"); break;
                case 1: printf(" (NON - Non-Confirmable)"); break;
                case 2: printf(" (ACK - Acknowledgment)"); break;
                case 3: printf(" (RST - Reset)"); break;
                default: printf(" (Tipo desconocido)"); break;
            }
            printf("\n");
            
            printf("   Message ID: %u\n", resp.message_id);
            printf("   Token: ");
            for (int i = 0; i < resp.tkl; i++) {
                printf("%02X ", resp.token[i]);
            }
            printf("\n");
            
            printf("   Opciones: %d\n", resp.option_count);
            for (int i = 0; i < resp.option_count; i++) {
                printf("     Opción %d: número=%d, longitud=%d\n", 
                       i+1, resp.options[i].number, resp.options[i].length);
            }
            
            if (resp.payload_len > 0) {
                printf("   Payload (%d bytes): %.*s\n", resp.payload_len, resp.payload_len, resp.payload);
            } else {
                printf("   Payload: (vacío)\n");
            }
            
            // Limpiar transacción
            clear_transaction(resp.message_id);

            for (int i = 0; i < resp.option_count; i++) {
                if (resp.options[i].value) free(resp.options[i].value);
            }
        } else {
            printf("<- Error: recibido %zd bytes (no es un mensaje CoAP válido)\n", recvd);
        }
    } else {
        printf("<- Timeout: sin respuesta dentro del tiempo límite\n");
        // Aquí podrías reintentar si fuera CON
    }

    close(sockfd);
    return 0;
}

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


