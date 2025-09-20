// udp_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>


#define SERVER_IP "127.0.0.1"
#define PORT 5683
#define BUF_SIZE 1024

int main()
{

    int sockfd;
    char buffer[BUF_SIZE];
    struct sockaddr_in server_addr;

    // Crear socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

    // Configurar dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Enviar mensaje
    char *msg = "Hola CoAP!";


    do
    {
        usleep(2000000);

        sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        socklen_t addr_len = sizeof(server_addr);
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
        buffer[n] = '\0';
        printf("Respuesta del servidor: %s\n", buffer);

        
    } while (1);
    
    
    // Recibir respuesta
    

    close(sockfd);
    return 0;
}
