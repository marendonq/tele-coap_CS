#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "model/message.h"

int main()
{

    //! Conexion
    // Crear socket
    int socketID;
    char buffer[1024];
    struct sockaddr_in server_addr;

    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketID < 0)
    {
        printf("Error de socket() => %d", socketID);
        return 0;
    }

    // Configurar direccion del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5683);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    CoapMessage msg;
    coap_message_init(&msg);

    // Logica para el manejo de mensajes
    msg.type = 0; // CON
    msg.code = 1; // GET
    msg.message_id = 0x1234;

    coap_add_option(&msg, 11, (unsigned char *)"test", 4);

    coap_set_payload(&msg, (unsigned char *)"Hello CoAP", 10);

    int len = coap_serialize(&msg, buffer, sizeof(buffer));

    sendto(socketID, &buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("Mensaje enviado\n");

    // sendto(sockfd, buffer, len, 0, (struct sockaddr *)&dest, sizeof(dest));

    // En recepcion

    // CoapMessage received;
    // coap_parse(&received, buffer, n);
    // printf("Code: %d, Payload: %.*s\n",
    //        received.code, received.payload_len, received.payload);
}
