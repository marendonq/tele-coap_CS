#include "coap_router.h"
#include <string.h>
#include <stdio.h>

static int handle_get(const coap_message_t* req, char* out_payload, size_t out_size) {
    // Ejemplo simple: devolver estado del recurso
    // En proyecto real, leerías sensores/estado según req->uri_path
    if (strncmp(req->uri_path, "/sensors/temp", req->uri_path_len) == 0) {
        snprintf(out_payload, out_size, "{\"temp_c\":%.1f}", 25.5);
    } else {
        snprintf(out_payload, out_size, "{\"status\":\"ok\"}");
    }
    return COAP_RESPONSE_CONTENT; // 2.05
}

static int handle_post(const coap_message_t* req, char* out_payload, size_t out_size) {
    // Simular recepción de datos (ya llega en req->payload)
    snprintf(out_payload, out_size, "{\"status\":\"ok\",\"message\":\"created/updated\"}");
    return COAP_RESPONSE_CHANGED; // 2.04
}

static int handle_put(const coap_message_t* req, char* out_payload, size_t out_size) {
    // Simular actualización de configuración
    snprintf(out_payload, out_size, "{\"status\":\"ok\",\"message\":\"updated\"}");
    return COAP_RESPONSE_CHANGED; // 2.04
}

static int handle_delete(const coap_message_t* req, char* out_payload, size_t out_size) {
    // Simular borrado
    snprintf(out_payload, out_size, "{\"status\":\"ok\",\"message\":\"deleted\"}");
    return COAP_RESPONSE_DELETED; // 2.02
}

int coap_router_handle_request(const coap_message_t* request,
                               uint8_t* out_code,
                               char* out_payload,
                               size_t out_payload_size) {
    if (!request || !out_code || !out_payload || out_payload_size == 0) return -1;

    // Default payload vacío
    out_payload[0] = '\0';

    switch (request->code) {
        case COAP_METHOD_GET:
            *out_code = (uint8_t)handle_get(request, out_payload, out_payload_size);
            return 0;
        case COAP_METHOD_POST:
            *out_code = (uint8_t)handle_post(request, out_payload, out_payload_size);
            return 0;
        case COAP_METHOD_PUT:
            *out_code = (uint8_t)handle_put(request, out_payload, out_payload_size);
            return 0;
        case COAP_METHOD_DELETE:
            *out_code = (uint8_t)handle_delete(request, out_payload, out_payload_size);
            return 0;
        default:
            // Método no soportado -> 4.05 Method Not Allowed (código 133), pero usamos 2.xx por simplicidad
            *out_code = COAP_RESPONSE_VALID; // 2.03
            snprintf(out_payload, out_payload_size, "{\"status\":\"unsupported\"}");
            return 0;
    }
}


