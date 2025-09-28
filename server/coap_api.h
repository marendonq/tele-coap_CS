#ifndef COAP_API_H
#define COAP_API_H

#include <stddef.h>
#include <stdint.h>
#include "coap_parser.h" 


#ifdef __cplusplus
extern "C" {
#endif

// Métodos CoAP básicos
#define COAP_METHOD_GET     1
#define COAP_METHOD_POST    2
#define COAP_METHOD_PUT     3
#define COAP_METHOD_DELETE  4

#define RESPONSE_BUFFER_SIZE 512

// Tipo de handler
// Funcion que recibe una request y un buffer en el que se debe guardar la respuesta
// El handler debe retornar diferente de 0 si algo sale mal.
typedef int (*coap_handler_fn)(const coap_message_t *msg, char *responseBuffer);

typedef struct {
    char uri[128];
    uint8_t method;
    coap_handler_fn handler;
} Route;


coap_handler_fn find_handler(const char* uri, uint8_t method);

// Registrar handler para una URI y método
int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn);

// Iniciar servidor
int coap_server_start(int port, const char *logFileName);

// Detener servidor
// void coap_server_stop();

#ifdef __cplusplus
}
#endif

#endif // COAP_API_H
