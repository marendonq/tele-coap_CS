#include "server.h"
#include "logger.h"
#include "coap_parser.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <cstdlib>

#define BUFFER_SIZE 1024

void start_server(int port, Logger &logger) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    uint8_t buffer[BUFFER_SIZE];

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
    printf("Servidor CoAP escuchando en puerto %d...\n", port);
    printf("Ruta: coap://<IP_PC>:%d/sensors/temp (espera POST)\n", port);
    logger.log("Servidor CoAP escuchando en puerto " + std::to_string(port));

    // 4. Loop principal: recibir y procesar mensajes CoAP
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

        printf("Mensaje recibido desde %s:%d (%zd bytes)\n", client_ip.c_str(), client_port, n);
        logger.log("Mensaje recibido desde " + client_ip + ":" + std::to_string(client_port) + 
                   " (" + std::to_string(n) + " bytes)");

        // Intentar parsear como mensaje CoAP
        coap_message_t request, response;
        if (parse_coap_message(buffer, n, &request) == 0) {
            printf("Mensaje CoAP parseado correctamente:\n");
            printf("  Versión: %d\n", request.ver);
            printf("  Tipo: %d\n", request.type);
            printf("  Token Length: %d\n", request.tkl);
            printf("  Código: %d\n", request.code);
            printf("  Message ID: %d\n", request.mid);
            
            logger.log("Mensaje CoAP parseado - Ver:" + std::to_string(request.ver) + 
                       " Tipo:" + std::to_string(request.type) + 
                       " Código:" + std::to_string(request.code));

            // Procesar según el método
            if (request.code == COAP_METHOD_POST) {
                printf("  Método: POST\n");
                
                if (request.payload && request.payload_len > 0) {
                    std::string payload_str((char*)request.payload, request.payload_len);
                    printf("  Payload: %s\n", payload_str.c_str());
                    logger.log("Payload recibido: " + payload_str);
                    
                    // Simular procesamiento de datos de temperatura
                    printf("[POST] /sensors/temp <- %s\n", payload_str.c_str());
                } else {
                    printf("  Payload: <vacío>\n");
                    logger.log("Payload vacío recibido");
                }

                // Crear respuesta CoAP
                const char *response_payload = "{\"status\":\"ok\",\"message\":\"Temperature data received\"}";
                if (create_coap_response(&request, &response, COAP_RESPONSE_CHANGED, 
                                       response_payload, strlen(response_payload)) == 0) {
                    
                    // Serializar respuesta
                    uint8_t response_buffer[BUFFER_SIZE];
                    size_t response_len = serialize_coap_message(&response, response_buffer, BUFFER_SIZE);
                    
                    if (response_len > 0) {
                        // Enviar respuesta
                        ssize_t sent = sendto(sockfd, response_buffer, response_len, 0,
                                            (struct sockaddr *)&client_addr, client_len);
                        
                        if (sent > 0) {
                            printf("Respuesta enviada (%zd bytes)\n", sent);
                            logger.log("Respuesta CoAP enviada (" + std::to_string(sent) + " bytes)");
                        } else {
                            logger.log("Error al enviar respuesta");
                        }
                    }
                    
                    free_coap_message(&response);
                }
            } else {
                printf("  Método no soportado: %d\n", request.code);
                logger.log("Método CoAP no soportado: " + std::to_string(request.code));
            }
            
            free_coap_message(&request);
        } else {
            // No es un mensaje CoAP válido, mostrar como datos raw
            printf("Datos raw recibidos (%zd bytes): ", n);
            for (ssize_t i = 0; i < n; i++) {
                printf("%02x ", buffer[i]);
            }
            printf("\n");
            logger.log("Datos raw recibidos (no CoAP válido)");
        }
    }

    close(sockfd);
}
