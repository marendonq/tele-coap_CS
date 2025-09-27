#include "coap_api.c"

int coap_server_start(int port);


void *handlerFunctionTempPost(coap_message_t *msg)
{
    data_store(msg->payload)
}
void *handlerFunctionTempGet(coap_message_t *msg)
{
    data_store_consult(msg->payload)
}

int coap_register_handler("/temp/esp32", COAP_METHOD_POST, &handlerFunctionTempGet);

int coap_register_handler("/consultarTemperatura", COAP_METHOD_GET, &handlerFunctionTempGet);
