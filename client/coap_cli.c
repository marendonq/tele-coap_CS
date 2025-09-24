#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../server/message.h"

static void add_uri_path_options(CoapMessage *msg, const char *path) {
    const char *p = path;
    while (*p == '/') p++;
    while (*p) {
        const char *start = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - start);
        if (len) coap_add_option(msg, 11, (const unsigned char*)start, (unsigned short)len);
        while (*p == '/') p++;
    }
}

int main(int argc, char **argv) {
    if (argc < 6) {
        fprintf(stderr, "Uso: %s <host> <puerto> <ruta> <get|post|put|delete> <con|non> [json_payload]\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);
    const char *path = argv[3];
    const char *method_str = argv[4];
    const char *mode_str = argv[5];
    const char *json_payload = (argc >= 7) ? argv[6] : NULL;

    int method_code = 1; // GET por defecto
    if (strcmp(method_str, "get") == 0) method_code = 1;
    else if (strcmp(method_str, "post") == 0) method_code = 2;
    else if (strcmp(method_str, "put") == 0) method_code = 3;
    else if (strcmp(method_str, "delete") == 0) method_code = 4;

    int confirmable = (strcmp(mode_str, "con") == 0) ? 1 : 0;

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_addr; memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "IP inválida: %s\n", host);
        close(sockfd);
        return 1;
    }

    unsigned char buffer[1024];
    CoapMessage msg; coap_message_init(&msg);

    msg.type = confirmable ? 0 : 1; // CON/NON
    msg.code = (unsigned char)method_code;
    msg.message_id = (unsigned short)(rand() & 0xFFFF);

    // Token simple de 2 bytes
    msg.tkl = 2; msg.token[0] = (unsigned char)(rand() & 0xFF); msg.token[1] = (unsigned char)(rand() & 0xFF);

    add_uri_path_options(&msg, path);

    if ((method_code == 2 || method_code == 3) && json_payload && strlen(json_payload) > 0) {
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

    ssize_t sent = sendto(sockfd, buffer, (size_t)len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) { perror("sendto"); close(sockfd); return 1; }
    printf("-> %s %s (%zd bytes)\n", method_str, path, sent);

    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0; // 1s timeout
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    unsigned char recv_buf[1024]; socklen_t addr_len = sizeof(server_addr);
    ssize_t recvd = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&server_addr, &addr_len);
    if (recvd > 0) {
        CoapMessage resp;
        if (coap_parse(&resp, recv_buf, (int)recvd) == 0) {
            printf("<- code=%d t=%d mid=%u len=%d\n", resp.code, resp.type, resp.message_id, resp.payload_len);
            if (resp.payload_len > 0) {
                printf("<- payload: %.*s\n", resp.payload_len, resp.payload);
            }
            for (int i = 0; i < resp.option_count; i++) {
                if (resp.options[i].value) free(resp.options[i].value);
            }
        } else {
            printf("<- recibido %zd bytes (no CoAP válido)\n", recvd);
        }
    } else {
        printf("(sin respuesta dentro del timeout)\n");
    }

    close(sockfd);
    return 0;
}


