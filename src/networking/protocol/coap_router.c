#include "coap_router.h"
#include "coap_api.h"
#include <string.h>
#include <stdio.h>

#define MAX_BUFFER 1024

int coap_router_handle_request(const coap_message_t *request, uint8_t *out_code, char *out_payload, size_t out_payload_size)
{
    if (!request || !out_code || !out_payload || out_payload_size == 0)
        return -1;

    out_payload[0] = '\0';

    coap_handler_fn handler = find_handler(request->uri_path, (uint8_t)request->code);

    if (handler == NULL)
    {
        // URI no encontrado - retornar 0 para que el servidor maneje el error
        snprintf(out_payload, out_payload_size, "URI no encontrado: %s", request->uri_path);
        *out_code = PAGE_NOT_FOUND;
        return 0; // Cambiado de -1 a 0 para que el servidor procese la respuesta
    }

    if (request->type != COAP_TYPE_CONFIRMABLE)
    {
        *out_code = COAP_RESPONSE_NO_REPLY;
        return 0;
    }
    else
    {
        if (handler(request, out_payload) == 0)
        {
            *out_code = coap_default_success_code(request->code);
            return 0;
        }
        else
        {
            *out_code = SERVER_ERROR;
            return -1;
        }
    }
    return -1;
}


