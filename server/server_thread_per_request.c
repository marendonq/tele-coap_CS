#include "server.h"
#include "logger.h"
#include "coap_parser.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define BUFFER_SIZE 1024

// Datos que se pasan al thread de procesamiento
typedef struct ThreadData {
    struct MessageData *msg_data;
    Logger *logger;
} ThreadData;

// Variables globales para estadísticas
atomic_int active_threads = 0;
atomic_int total_messages_processed = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Función para obtener ID del thread como número
unsigned long get_thread_id() {
    return (unsigned long)pthread_self();
}

// Función para procesar un mensaje individual (compatible con pthread)
void* process_message(void* arg) {
    ThreadData *thread_data = (ThreadData*)arg;
    struct MessageData *msg_data = thread_data->msg_data;
    Logger *logger = thread_data->logger;
    
    const uint8_t* buffer = msg_data->buffer;
    ssize_t n = msg_data->len;
    const struct sockaddr_in* client_addr = &msg_data->client_addr;
    socklen_t client_len = msg_data->client_len;
    int sockfd = msg_data->sockfd;

    // Incrementar contador de threads activos
    atomic_fetch_add(&active_threads, 1);
    unsigned long thread_id = get_thread_id();

    // IP y puerto del cliente
    char *client_ip = inet_ntoa(client_addr->sin_addr);
    int client_port = ntohs(client_addr->sin_port);

    printf("[Thread %lu] Mensaje recibido desde %s:%d (%zd bytes) [Activos: %d]\n", 
           thread_id, client_ip, client_port, n, atomic_load(&active_threads));
    
    char log_msg[512];
    snprintf(log_msg, sizeof(log_msg), "Mensaje recibido desde %s:%d (%zd bytes)", 
             client_ip, client_port, n);
    logger_log(logger, log_msg);

    // Intentar parsear como mensaje CoAP
    coap_message_t request, response;
    if (parse_coap_message(buffer, n, &request) == 0) {
        printf("[Thread %lu] Mensaje CoAP parseado correctamente:\n", thread_id);
        printf("  Versión: %d\n", request.ver);
        printf("  Tipo: %d\n", request.type);
        printf("  Token Length: %d\n", request.tkl);
        printf("  Código: %d\n", request.code);
        printf("  Message ID: %d\n", request.mid);
        
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "Mensaje CoAP parseado - Ver:%d Tipo:%d Código:%d", 
                request.ver, request.type, request.code);
        logger_log(logger, log_msg);

        // Procesar según el método
        if (request.code == COAP_METHOD_POST) {
            printf("[Thread %lu] Método: POST\n", thread_id);
            
            if (request.payload && request.payload_len > 0) {
                char payload_str[256];
                size_t payload_len = (request.payload_len < sizeof(payload_str) - 1) ? 
                                    request.payload_len : sizeof(payload_str) - 1;
                memcpy(payload_str, request.payload, payload_len);
                payload_str[payload_len] = '\0';
                
                printf("[Thread %lu] Payload: %s\n", thread_id, payload_str);
                
                char log_msg[512];
                snprintf(log_msg, sizeof(log_msg), "Payload recibido: %s", payload_str);
                logger_log(logger, log_msg);
                
                // Simular procesamiento de datos de temperatura
                printf("[Thread %lu] [POST] /sensors/temp <- %s\n", thread_id, payload_str);
            } else {
                printf("[Thread %lu] Payload: <vacío>\n", thread_id);
                logger_log(logger, "Payload vacío recibido");
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
                                        (struct sockaddr *)client_addr, client_len);
                    
                    if (sent > 0) {
                        printf("[Thread %lu] Respuesta enviada (%zd bytes)\n", thread_id, sent);
                        char log_msg[256];
                        snprintf(log_msg, sizeof(log_msg), "Respuesta CoAP enviada (%zd bytes)", sent);
                        logger_log(logger, log_msg);
                    } else {
                        logger_log(logger, "Error al enviar respuesta");
                    }
                }
                
                free_coap_message(&response);
            }
        } else {
            printf("[Thread %lu] Método no soportado: %d\n", thread_id, request.code);
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "Método CoAP no soportado: %d", request.code);
            logger_log(logger, log_msg);
        }
        
        free_coap_message(&request);
    } else {
        // No es un mensaje CoAP válido, mostrar como datos raw
        printf("[Thread %lu] Datos raw recibidos (%zd bytes): ", thread_id, n);
        for (ssize_t i = 0; i < n; i++) {
            printf("%02x ", buffer[i]);
        }
        printf("\n");
        logger_log(logger, "Datos raw recibidos (no CoAP válido)");
    }

    // Incrementar contador de mensajes procesados
    atomic_fetch_add(&total_messages_processed, 1);
    
    // Decrementar contador de threads activos
    atomic_fetch_sub(&active_threads, 1);
    
    printf("[Thread %lu] Procesamiento completado [Total procesados: %d]\n", 
           thread_id, atomic_load(&total_messages_processed));
    
    // Liberar memoria
    free(msg_data);
    free(thread_data);
    
    return NULL;
}

void start_server(int port, Logger *logger) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    uint8_t buffer[BUFFER_SIZE];

    // 1. Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        logger_log(logger, "Error al crear socket");
        perror("socket");
        return;
    }
    logger_log(logger, "Socket UDP inicializado correctamente");

    // 2. Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // 3. Enlazar socket al puerto
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        logger_log(logger, "Error en bind");
        perror("bind");
        close(sockfd);
        return;
    }
    printf("Servidor CoAP Thread-per-Request escuchando en puerto %d...\n", port);
    printf("Ruta: coap://<IP_PC>:%d/sensors/temp (espera POST)\n", port);
    printf("Modo: Thread-per-Request (sin cola FIFO)\n");
    
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "Servidor CoAP Thread-per-Request escuchando en puerto %d", port);
    logger_log(logger, log_msg);

    // 4. Loop principal: recibir mensajes y crear thread para cada uno
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        client_len = sizeof(client_addr);

        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                             (struct sockaddr *)&client_addr, &client_len);

        if (n < 0) {
            logger_log(logger, "Error al recibir datagrama");
            perror("recvfrom");
            continue;
        }

        // Crear estructura de datos del mensaje
        struct MessageData *msg_data = (struct MessageData*)malloc(sizeof(struct MessageData));
        if (!msg_data) {
            fprintf(stderr, "Error al asignar memoria para MessageData\n");
            continue;
        }
        
        memcpy(msg_data->buffer, buffer, n);
        msg_data->len = n;
        msg_data->client_addr = client_addr;
        msg_data->client_len = client_len;
        msg_data->sockfd = sockfd;

        // Crear estructura para pasar datos al thread
        ThreadData *thread_data = (ThreadData*)malloc(sizeof(ThreadData));
        if (!thread_data) {
            fprintf(stderr, "Error al asignar memoria para thread_data\n");
            free(msg_data);
            continue;
        }
        
        thread_data->msg_data = msg_data;
        thread_data->logger = logger;

        // Crear thread para procesar el mensaje    
        pthread_t thread;
        if (pthread_create(&thread, NULL, process_message, (void*)thread_data) != 0) {
            fprintf(stderr, "Error al crear thread\n");
            free(msg_data);
            free(thread_data);
            continue;
        }
        
        // Detach thread para que se limpie automáticamente
        pthread_detach(thread);

        printf("Thread creado para mensaje [Threads activos: %d, Total procesados: %d]\n", 
               atomic_load(&active_threads), atomic_load(&total_messages_processed));
    }

    close(sockfd);
}
