#ifndef HANDLERS_H
#define HANDLERS_H

#include "coap_api.h"

int HandlerFunctionTempPost(const coap_message_t *msg, char *responseBuffer);
int HandlerFunctionTempGet(const coap_message_t *msg, char *responseBuffer);
int HandlerFunctionTempPut(const coap_message_t *msg, char *responseBuffer);
int HandlerFunctionTempDelete(const coap_message_t *msg, char *responseBuffer);

#endif



