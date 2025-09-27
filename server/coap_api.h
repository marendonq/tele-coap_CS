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

// // Request simplificado
// typedef struct {
//     const char* uri;
//     uint8_t method;
//     const uint8_t* payload;
//     size_t payload_len;
// } CoapRequest;

// // Response simplificado
// typedef struct {
//     uint8_t code;       // Ej: 69 = 2.05 Content
//     char* payload;      // buffer destino
//     size_t payload_size;
// } CoapResponse;

// Tipo de handler
typedef void (*coap_handler_fn)(const coap_message_t *msg);

// Registrar handler para una URI y método
int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn);

// Iniciar servidor
int coap_server_start(int port);

// Detener servidor
void coap_server_stop();

#ifdef __cplusplus
}
#endif

#endif // COAP_API_H
