#include "coap_parser.h"
#include <string.h>
#include <stdlib.h>

// Función para leer un número de longitud variable (RFC 7252)
static uint32_t read_var_length(const uint8_t **data, size_t *remaining) {
    if (*remaining == 0) return 0;
    
    uint8_t first_byte = **data;
    (*data)++;
    (*remaining)--;
    
    if (first_byte < 13) {
        return first_byte;
    } else if (first_byte == 13) {
        if (*remaining < 1) return 0;
        uint8_t second = **data;
        (*data)++;
        (*remaining)--;
        return second + 13;
    } else if (first_byte == 14) {
        if (*remaining < 2) return 0;
        uint16_t val = (**data << 8) | *(*data + 1);
        (*data) += 2;
        (*remaining) -= 2;
        return val + 269;
    } else {
        // first_byte == 15, reservado
        return 0;
    }
}

int parse_coap_message(const uint8_t *data, size_t len, coap_message_t *msg) {
    if (!data || !msg || len < 4) {
        return -1; // Error: datos insuficientes
    }
    
    memset(msg, 0, sizeof(coap_message_t));
    
    // Parsear header CoAP
    uint8_t byte0 = data[0];
    msg->ver = (byte0 >> 6) & 0x03;
    msg->type = (byte0 >> 4) & 0x03;
    msg->tkl = byte0 & 0x0F;
    
    msg->code = data[1];
    msg->mid = (data[2] << 8) | data[3];
    
    size_t offset = 4;
    
    // Parsear token
    if (msg->tkl > 0) {
        if (offset + msg->tkl > len) return -1;
        memcpy(msg->token, data + offset, msg->tkl);
        offset += msg->tkl;
    }
    
    // Parsear opciones (simplificado - solo buscamos URI_PATH y CONTENT_FORMAT)
    while (offset < len) {
        if (offset >= len) break;
        
        uint8_t option_byte = data[offset++];
        if (option_byte == 0xFF) {
            // Payload marker
            offset++;
            break;
        }
        
        // Parsear delta y length
        uint32_t delta = read_var_length(&data, &offset);
        uint32_t length = read_var_length(&data, &offset);
        
        if (offset + length > len) break;
        
        // Solo procesamos opciones importantes
        if (delta == COAP_OPTION_URI_PATH || delta == COAP_OPTION_CONTENT_FORMAT) {
            // Por simplicidad, solo guardamos que encontramos estas opciones
            // En una implementación completa, guardaríamos los valores
        }
        
        offset += length;
    }
    
    // Parsear payload
    if (offset < len) {
        msg->payload_len = len - offset;
        msg->payload = (uint8_t*)malloc(msg->payload_len);
        if (msg->payload) {
            memcpy(msg->payload, data + offset, msg->payload_len);
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
    
    memset(response, 0, sizeof(coap_message_t));
    
    // Configurar respuesta
    response->ver = request->ver;
    response->type = COAP_TYPE_ACKNOWLEDGMENT; // ACK para CON, NON para NON
    response->tkl = request->tkl;
    response->code = code;
    response->mid = request->mid;
    
    // Copiar token
    if (request->tkl > 0) {
        memcpy(response->token, request->token, request->tkl);
    }
    
    // Configurar payload
    if (payload && payload_len > 0) {
        response->payload = (uint8_t*)malloc(payload_len);
        if (response->payload) {
            memcpy(response->payload, payload, payload_len);
            response->payload_len = payload_len;
        }
    }
    
    return 0;
}

size_t serialize_coap_message(const coap_message_t *msg, uint8_t *buffer, size_t buffer_size) {
    if (!msg || !buffer || buffer_size < 4) return 0;
    
    size_t offset = 0;
    
    // Escribir header
    uint8_t byte0 = (msg->ver << 6) | (msg->type << 4) | (msg->tkl & 0x0F);
    buffer[offset++] = byte0;
    buffer[offset++] = msg->code;
    buffer[offset++] = (msg->mid >> 8) & 0xFF;
    buffer[offset++] = msg->mid & 0xFF;
    
    // Escribir token
    if (msg->tkl > 0 && offset + msg->tkl <= buffer_size) {
        memcpy(buffer + offset, msg->token, msg->tkl);
        offset += msg->tkl;
    }
    
    // Escribir payload marker y payload
    if (msg->payload && msg->payload_len > 0) {
        if (offset + 1 + msg->payload_len <= buffer_size) {
            buffer[offset++] = 0xFF; // Payload marker
            memcpy(buffer + offset, msg->payload, msg->payload_len);
            offset += msg->payload_len;
        }
    }
    
    return offset;
}
