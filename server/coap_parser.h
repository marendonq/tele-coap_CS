#ifndef COAP_PARSER_H
#define COAP_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Estructuras básicas de CoAP
typedef struct {
    uint8_t ver;        // Versión (2 bits)
    uint8_t type;       // Tipo de mensaje (2 bits)
    uint8_t tkl;        // Longitud del token (4 bits)
    uint8_t code;       // Código de método/respuesta (8 bits)
    uint16_t mid;       // Message ID (16 bits)
    uint8_t token[8];   // Token (hasta 8 bytes)
    uint8_t *options;   // Opciones
    size_t options_len; // Longitud de opciones
    uint8_t *payload;   // Payload
    size_t payload_len; // Longitud del payload

    // Campos derivados para facilitar el ruteo
    char uri_path[128];     // Uri-Path concatenado, p. ej. "/sensors/temp"
    size_t uri_path_len;    // Longitud del uri_path
    int content_format;     // Content-Format si viene presente (ej. 50=application/json), -1 si no presente
    
} coap_message_t;

// Códigos de método CoAP
#define COAP_METHOD_GET     1
#define COAP_METHOD_POST    2
#define COAP_METHOD_PUT     3
#define COAP_METHOD_DELETE  4

// Códigos de respuesta CoAP
#define COAP_RESPONSE_CREATED       65  // 2.01
#define COAP_RESPONSE_DELETED       66  // 2.02
#define COAP_RESPONSE_VALID        67  // 2.03
#define COAP_RESPONSE_CHANGED      68  // 2.04
#define COAP_RESPONSE_CONTENT      69  // 2.05

// Tipos de mensaje CoAP
#define COAP_TYPE_CONFIRMABLE    0
#define COAP_TYPE_NON_CONFIRMABLE 1
#define COAP_TYPE_ACKNOWLEDGMENT  2
#define COAP_TYPE_RESET          3

// Opciones CoAP
#define COAP_OPTION_URI_PATH      11
#define COAP_OPTION_CONTENT_FORMAT 12

// Funciones del parser
int parse_coap_message(const uint8_t *data, size_t len, coap_message_t *msg);
void free_coap_message(coap_message_t *msg);
int create_coap_response(const coap_message_t *request, coap_message_t *response, 
                        uint8_t code, const char *payload, size_t payload_len);
size_t serialize_coap_message(const coap_message_t *msg, uint8_t *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif
