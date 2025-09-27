#ifndef COAP_ROUTER_H
#define COAP_ROUTER_H

#include <stddef.h>
#include <stdint.h>
#include "coap_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*coap_handler_fn)(const coap_message_t *msg);

typedef struct {
    char uri[128];
    uint8_t method;
    coap_handler_fn handler;
} Route;

// Maneja un request CoAP, devuelve código de respuesta y payload JSON opcional.
// Retorna 0 en éxito, -1 en error.
int coap_router_handle_request(const coap_message_t* request,
                               uint8_t* out_code,
                               char* out_payload,
                               size_t out_payload_size);

int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn);




#ifdef __cplusplus
}
#endif

#endif


