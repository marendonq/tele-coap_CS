#include "handlers.h"
#include "data_store.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Función simple para validar JSON básico
int is_valid_json(const char *str) {
    if (!str || strlen(str) == 0) return 0;
    
    // Buscar llaves de apertura y cierre
    int open_braces = 0;
    int close_braces = 0;
    
    for (int i = 0; str[i]; i++) {
        if (str[i] == '{') open_braces++;
        else if (str[i] == '}') close_braces++;
    }
    
    // JSON válido debe tener llaves balanceadas
    return (open_braces > 0 && open_braces == close_braces);
}

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
    
    // Validar que el payload sea JSON válido
    if (!is_valid_json(tmp)) {
        snprintf(responseBuffer, 512, "Error: Payload no es JSON válido. Recibido: '%s'", tmp);
        return -1;
    }
    
    if (data_store_set(msg->uri_path, tmp) != 0)
    {
        snprintf(responseBuffer, 512, "Error al persistir payload (%zu bytes)", msg->payload_len);
        return -1;
    }
    snprintf(responseBuffer, 512, "JSON válido recibido y guardado (%zu bytes)", msg->payload_len);
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
    
    // Validar que el payload sea JSON válido
    if (!is_valid_json(tmp)) {
        snprintf(responseBuffer, 512, "Error: Payload no es JSON válido. Recibido: '%s'", tmp);
        return -1;
    }
    
    if (data_store_set(msg->uri_path, tmp) != 0)
    {
        snprintf(responseBuffer, 512, "Error al actualizar payload (%zu bytes)", msg->payload_len);
        return -1;
    }
    snprintf(responseBuffer, 512, "JSON válido actualizado (%zu bytes)", msg->payload_len);
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


