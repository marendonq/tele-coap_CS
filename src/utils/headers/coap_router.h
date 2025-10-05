#ifndef COAP_ROUTER_H
#define COAP_ROUTER_H

#include <stddef.h>
#include <stdint.h>
#include "coap_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

int coap_router_handle_request(const coap_message_t* request,
                               uint8_t* out_code,
                               char* out_payload,
                               size_t out_payload_size);

#ifdef __cplusplus
}
#endif

#endif



