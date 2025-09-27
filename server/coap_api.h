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

typedef struct {
    char uri[128];
    uint8_t method;
    coap_handler_fn handler;
} Route;

// Tipo de handler
typedef void (*coap_handler_fn)(const coap_message_t *msg);

// Registrar handler para una URI y método
int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn);

// Iniciar servidor
int coap_server_start(int port, char *logFileName);

// Detener servidor
// void coap_server_stop();

#ifdef __cplusplus
}
#endif

#endif // COAP_API_H
