// udp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "model/message.h"

#define PORT 5683 // puerto estándar de CoAP
#define BUF_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUF_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Enlazar el socket al puerto
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    printf("Servidor UDP escuchando en puerto %d...\n", PORT);
    
    CoapMessage recvMessage;
    // Recibir mensajes
    while (1)
    {
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        buffer[n] = '\0';
        
        coap_parse(&recvMessage, buffer, BUF_SIZE);

        // respond with ack

        // sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}
