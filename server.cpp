#include "server.h"
#include "logger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

#define BUFFER_SIZE 1024

void start_server(int port, Logger &logger) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // 1. Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        logger.log("Error al crear socket");
        perror("socket");
        return;
    }
    logger.log("Socket UDP inicializado correctamente");

    // 2. Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 3. Enlazar socket al puerto
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        logger.log("Error en bind");
        perror("bind");
        close(sockfd);
        return;
    }
    printf("Servidor UDP escuchando en puerto %d...\n", port);
    logger.log("Servidor UDP escuchando en puerto " + std::to_string(port));

    // 4. Loop principal: recibir mensajes
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        client_len = sizeof(client_addr);

        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr, &client_len);

        if (n < 0) {
            logger.log("Error al recibir datagrama");
            perror("recvfrom");
            continue;
        }

        // IP y puerto del cliente
        std::string client_ip = inet_ntoa(client_addr.sin_addr);
        int client_port = ntohs(client_addr.sin_port);

        printf("Mensaje recibido desde %s:%d\n", client_ip.c_str(), client_port);
        logger.log("Mensaje recibido desde " + client_ip + ":" + std::to_string(client_port));

        // Mostrar bytes recibidos en hexadecimal
        printf("Bytes recibidos (%zd): ", n);
        for (ssize_t i = 0; i < n; i++) {
            printf("%02x ", (unsigned char)buffer[i]);
        }
        printf("\n");

        // Guardar también contenido en el log (si es imprimible)
        logger.log("Contenido: " + std::string(buffer, buffer + n));
    }

    close(sockfd);
}
