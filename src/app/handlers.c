#include "handlers.h"
#include "data_store.h"
#include <string.h>
#include <stdio.h>

int HandlerFunctionTempPost(const coap_message_t *msg, char *responseBuffer)
{
    if (msg == NULL || responseBuffer == NULL)
    {
        return -1;
    }
    if (!msg->payload || msg->payload_len == 0)
    {
        snprintf(responseBuffer, 512, "Sin payload que guardar");
        return 0;
    }
    char tmp[512];
    size_t copy_len = msg->payload_len < (sizeof(tmp) - 1) ? msg->payload_len : (sizeof(tmp) - 1);
    memcpy(tmp, msg->payload, copy_len);
    tmp[copy_len] = '\0';
    if (data_store_set(msg->uri_path, tmp) != 0)
    {
        snprintf(responseBuffer, 512, "Error al persistir payload (%zu bytes)", msg->payload_len);
        return -1;
    }
    snprintf(responseBuffer, 512, "Payload recibido y guardado (%zu bytes)", msg->payload_len);
    return 0;
}

int HandlerFunctionTempGet(const coap_message_t *msg, char *responseBuffer)
{
    if (msg == NULL || responseBuffer == NULL)
    {
        return -1;
    }
    int n = data_store_get(msg->uri_path, responseBuffer, 512);
    if (n < 0)
    {
        snprintf(responseBuffer, 512, "Error al leer del data store");
        return -1;
    }
    if (n == 0)
    {
        snprintf(responseBuffer, 512, "No hay datos para %s", msg->uri_path);
    }
    return 0;
}

int HandlerFunctionTempPut(const coap_message_t *msg, char *responseBuffer)
{
    if (msg == NULL || responseBuffer == NULL)
    {
        return -1;
    }
    if (!msg->payload || msg->payload_len == 0)
    {
        snprintf(responseBuffer, 512, "Sin payload que actualizar");
        return 0;
    }
    char tmp[512];
    size_t copy_len = msg->payload_len < (sizeof(tmp) - 1) ? msg->payload_len : (sizeof(tmp) - 1);
    memcpy(tmp, msg->payload, copy_len);
    tmp[copy_len] = '\0';
    if (data_store_set(msg->uri_path, tmp) != 0)
    {
        snprintf(responseBuffer, 512, "Error al actualizar payload (%zu bytes)", msg->payload_len);
        return -1;
    }
    snprintf(responseBuffer, 512, "Payload actualizado (%zu bytes)", msg->payload_len);
    return 0;
}

int HandlerFunctionTempDelete(const coap_message_t *msg, char *responseBuffer)
{
    if (msg == NULL || responseBuffer == NULL)
    {
        return -1;
    }
    if (data_store_delete(msg->uri_path) != 0)
    {
        snprintf(responseBuffer, 512, "Error al eliminar %s", msg->uri_path);
        return -1;
    }
    snprintf(responseBuffer, 512, "Recurso eliminado: %s", msg->uri_path);
    return 0;
}


