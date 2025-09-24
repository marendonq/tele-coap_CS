#ifndef COAPMESSAGE
#define COAPMESSAGE

#define MAX_TOKEN_LEN 8
#define MAX_OPTIONS 16
#define MAX_PAYLOAD 256

typedef struct
{
    unsigned short number;
    unsigned short length;
    unsigned char *value;
} CoapOption;

typedef struct
{
    unsigned char version;
    unsigned char type;
    unsigned char tkl;
    unsigned char code;
    unsigned short message_id;

    unsigned char token[MAX_TOKEN_LEN];
    int token_len;

    CoapOption options[MAX_OPTIONS];
    int option_count;

    unsigned char payload[MAX_PAYLOAD];
    int payload_len;
} CoapMessage;

int coap_message_init(CoapMessage *msg);

int coap_add_option(CoapMessage *msg, unsigned short number, const unsigned char *value, unsigned short length);

int coap_set_payload(CoapMessage *msg, const unsigned char *data, int len);

int coap_serialize(CoapMessage *msg, unsigned char *buffer, int buf_size);

int coap_parse(CoapMessage *msg, const unsigned char *buffer, int len);

#endif
