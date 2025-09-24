#include "coap_parser.h"
#include "message.h"
#include <string.h>
#include <stdlib.h>

// Adaptador: convertir de CoapMessage a coap_message_t
static void convert_from_coap_message(const CoapMessage *src, coap_message_t *dst) {
    memset(dst, 0, sizeof(coap_message_t));
    
    dst->ver = src->version;
    dst->type = src->type;
    dst->tkl = src->tkl;
    dst->code = src->code;
    dst->mid = src->message_id;
    
    // Copiar token
    if (src->tkl > 0) {
        memcpy(dst->token, src->token, src->tkl);
    }
    
    // Copiar payload
    if (src->payload_len > 0) {
        dst->payload_len = src->payload_len;
        dst->payload = (uint8_t*)malloc(src->payload_len);
        if (dst->payload) {
            memcpy(dst->payload, src->payload, src->payload_len);
        }
    }
}

// Adaptador: convertir de coap_message_t a CoapMessage
static void convert_to_coap_message(const coap_message_t *src, CoapMessage *dst) {
    coap_message_init(dst);
    
    dst->version = src->ver;
    dst->type = src->type;
    dst->tkl = src->tkl;
    dst->code = src->code;
    dst->message_id = src->mid;
    
    // Copiar token
    if (src->tkl > 0) {
        memcpy(dst->token, src->token, src->tkl);
        dst->token_len = src->tkl;
    }
    
    // Copiar payload
    if (src->payload_len > 0) {
        coap_set_payload(dst, src->payload, src->payload_len);
    }
}

int parse_coap_message(const uint8_t *data, size_t len, coap_message_t *msg) {
    if (!data || !msg || len < 4) {
        return -1; // Error: datos insuficientes
    }
    
    // Usar el parser del módulo CoAP
    CoapMessage parsed_coap_message;
    if (coap_parse(&parsed_coap_message, data, len) != 0) {
        return -1; // Error en el parseo
    }
    
    // Convertir al formato del servidor
    convert_from_coap_message(&parsed_coap_message, msg);
    
    // Liberar memoria de opciones del mensaje parseado
    for (int i = 0; i < parsed_coap_message.option_count; i++) {
        if (parsed_coap_message.options[i].value) {
            free(parsed_coap_message.options[i].value);
        }
    }
    
    return 0; // Éxito
}

void free_coap_message(coap_message_t *msg) {
    if (msg) {
        if (msg->payload) {
            free(msg->payload);
            msg->payload = NULL;
        }
        if (msg->options) {
            free(msg->options);
            msg->options = NULL;
        }
    }
}

int create_coap_response(const coap_message_t *request, coap_message_t *response, 
                        uint8_t code, const char *payload, size_t payload_len) {
    if (!request || !response) return -1;
    
    // Convertir request a CoapMessage
    CoapMessage request_coap_message;
    convert_to_coap_message(request, &request_coap_message);
    
    // Crear respuesta usando el módulo CoAP
    CoapMessage response_coap_message;
    coap_message_init(&response_coap_message);
    
    // Configurar respuesta
    response_coap_message.version = request_coap_message.version;
    response_coap_message.type = COAP_TYPE_ACKNOWLEDGMENT; // ACK para CON, NON para NON
    response_coap_message.tkl = request_coap_message.tkl;
    response_coap_message.code = code;
    response_coap_message.message_id = request_coap_message.message_id;
    
    // Copiar token
    if (request_coap_message.tkl > 0) {
        memcpy(response_coap_message.token, request_coap_message.token, request_coap_message.tkl);
        response_coap_message.token_len = request_coap_message.tkl;
    }
    
    // Configurar payload
    if (payload && payload_len > 0) {
        coap_set_payload(&response_coap_message, (const unsigned char*)payload, payload_len);
    }
    
    // Convertir respuesta al formato del servidor
    convert_from_coap_message(&response_coap_message, response);
    
    // Liberar memoria de opciones
    for (int i = 0; i < request_coap_message.option_count; i++) {
        if (request_coap_message.options[i].value) {
            free(request_coap_message.options[i].value);
        }
    }
    
    return 0;
}

size_t serialize_coap_message(const coap_message_t *msg, uint8_t *buffer, size_t buffer_size) {
    if (!msg || !buffer || buffer_size < 4) return 0;
    
    // Convertir a CoapMessage
    CoapMessage coap_message;
    convert_to_coap_message(msg, &coap_message);
    
    // Serializar usando el módulo CoAP
    int len = coap_serialize(&coap_message, buffer, buffer_size);
    
    // Liberar memoria de opciones
    for (int i = 0; i < coap_message.option_count; i++) {
        if (coap_message.options[i].value) {
            free(coap_message.options[i].value);
        }
    }
    
    return (len > 0) ? len : 0;
}
