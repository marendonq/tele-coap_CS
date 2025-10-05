#ifndef COAP_PARSER_H
#define COAP_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t ver;
    uint8_t type;
    uint8_t tkl;
    uint8_t code;
    uint16_t mid;
    uint8_t token[8];
    uint8_t *options;
    size_t options_len;
    uint8_t *payload;
    size_t payload_len;

    char uri_path[128];
    size_t uri_path_len;
    int content_format;
    
} coap_message_t;

#define COAP_METHOD_GET     1
#define COAP_METHOD_POST    2
#define COAP_METHOD_PUT     3
#define COAP_METHOD_DELETE  4

#define COAP_RESPONSE_CREATED       65
#define COAP_RESPONSE_DELETED       66
#define COAP_RESPONSE_VALID        67
#define COAP_RESPONSE_CHANGED      68
#define COAP_RESPONSE_CONTENT      69
#define PAGE_NOT_FOUND              132
#define SERVER_ERROR              160
#define COAP_RESPONSE_NO_REPLY 199

#define COAP_TYPE_CONFIRMABLE    0
#define COAP_TYPE_NON_CONFIRMABLE 1
#define COAP_TYPE_ACKNOWLEDGMENT  2
#define COAP_TYPE_RESET          3

#define COAP_OPTION_URI_PATH      11
#define COAP_OPTION_CONTENT_FORMAT 12

int coap_default_success_code(uint8_t method);
int parse_coap_message(const uint8_t *data, size_t len, coap_message_t *msg);
void free_coap_message(coap_message_t *msg);
int create_coap_response(const coap_message_t *request, coap_message_t *response, 
                        uint8_t code, const char *payload, size_t payload_len);
size_t serialize_coap_message(const coap_message_t *msg, uint8_t *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif


