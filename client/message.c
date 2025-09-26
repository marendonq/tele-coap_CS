#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message.h"

// Inicializa un mensaje vacío
int coap_message_init(CoapMessage *msg)
{
    msg->version = 1;
    msg->type = 0;
    msg->tkl = 0;
    msg->code = 0;
    msg->message_id = 0;
    msg->token_len = 0;
    msg->option_count = 0;
    msg->payload_len = 0;
    return 0;
}

// Agregar opción
int coap_add_option(CoapMessage *msg, unsigned short number, const unsigned char *value, unsigned short length)
{
    if (msg->option_count >= MAX_OPTIONS)
    {
        return -1;
    }

    msg->options[msg->option_count].number = number;
    msg->options[msg->option_count].length = length;
    msg->options[msg->option_count].value = malloc(length);
    memcpy(msg->options[msg->option_count].value, value, length);
    msg->option_count++;
    return 0;
}

// Agregar payload
int coap_set_payload(CoapMessage *msg, const unsigned char *data, int len)
{
    if (len > MAX_PAYLOAD)
    {
        return -1;
    }
    memcpy(msg->payload, data, len);
    msg->payload_len = len;
    return 0;
}

int coap_serialize(CoapMessage *msg, unsigned char *buffer, int buf_size)
{
    int offset = 0;

    memset(buffer, 0, sizeof(buffer));

    // Valida que el buffer tenga el tamaño minimo para el mensaje mas pequeño
    if (buf_size < 4)
        return -1;

    // Lengths 9-15 are reserved, MUST NOT be sent
    if (msg->tkl > 8)
    {
        return -1;
    }

    buffer[offset++] = (msg->version << 6) | (msg->type << 4) | (msg->tkl & 0x0F);
    buffer[offset++] = msg->code;
    buffer[offset++] = (msg->message_id >> 8) & 0xFF;
    buffer[offset++] = msg->message_id & 0xFF;

    if (msg->tkl > 0)
    {
        memcpy(&buffer[offset], msg->token, msg->tkl);
        offset += msg->tkl;
    }

    int last_number = 0;
    for (int i = 0; i < msg->option_count; i++)
    {

        //! TODO; implementar un delta variante
        int delta = msg->options[i].number - last_number;
        int len = msg->options[i].length;
        buffer[offset++] = (delta << 4) | (len & 0x0F);
        memcpy(&buffer[offset], msg->options[i].value, len);
        offset += len;
        last_number = msg->options[i].number;
    }

    if (msg->payload_len > 0)
    {
        buffer[offset++] = 0xFF;
        memcpy(&buffer[offset], msg->payload, msg->payload_len);
        offset += msg->payload_len;
    }

    return offset;
}

int coap_parse(CoapMessage *msg, const unsigned char *buffer, int len)
{
    if (len < 4)
        return -1;

    msg->version = (buffer[0] & 0xC0) >> 6;
    msg->type = (buffer[0] & 0x30) >> 4;
    msg->tkl = buffer[0] & 0x0F;
    msg->code = buffer[1];
    msg->message_id = (buffer[2] << 8) | buffer[3];

    int offset = 4;
    msg->token_len = msg->tkl;
    if (msg->token_len > 0)
    {
        memcpy(msg->token, &buffer[offset], msg->token_len);
        offset += msg->token_len;
    }

    msg->option_count = 0;
    int last_number = 0;

    while (offset < len)
    {
        if (buffer[offset] == 0xFF)
        { // payload marker
            offset++;
            msg->payload_len = len - offset;
            memcpy(msg->payload, &buffer[offset], msg->payload_len);
            return 0;
        }

        unsigned char delta = buffer[offset] >> 4;
        unsigned char optlen = buffer[offset] & 0x0F;
        offset++;

        int number = last_number + delta;
        last_number = number;

        msg->options[msg->option_count].number = number;
        msg->options[msg->option_count].length = optlen;
        msg->options[msg->option_count].value = malloc(optlen);
        memcpy(msg->options[msg->option_count].value, &buffer[offset], optlen);
        offset += optlen;
        msg->option_count++;
    }

    msg->payload_len = 0; // no payload
    return 0;
}
