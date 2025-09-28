#include "data_store.h"
#include "coap_api.h"
#include <stdlib.h> // atoi
#include <stdio.h>  // printf

char responseBuffer[RESPONSE_BUFFER_SIZE];

// Ejemplo de handler
int guardarDatos(const coap_message_t *msg, char *responseBuffer)
{

    printf("[HANDLER] POST /sensors/temp\n");

    if (msg->payload && msg->payload_len > 0)
    {
        // Asumimos que es texto legible
        printf("Payload recibido (%zu bytes): %.*s\n",
               msg->payload_len,
               (int)msg->payload_len,
               msg->payload);
    }
    else
    {
        printf("Payload vacío\n");
    }

    snprintf(responseBuffer, RESPONSE_BUFFER_SIZE,"Payload recibido con %zu bytes", msg->payload_len);

    return 0;
}

int main(int argc, char *argv[])
{
    int port = 5683;                    // Puerto por defecto
    const char *logFile = "server.log"; // Archivo log por defecto

    // Leer argumentos de línea de comandos
    if (argc > 1)
    {
        port = atoi(argv[1]); // Primer argumento = puerto
    }
    if (argc > 2)
    {
        logFile = argv[2]; // Segundo argumento = archivo de log
    }

    coap_register_handler("/sensors/temp", COAP_METHOD_POST, guardarDatos);

    coap_server_start(port, logFile);

    // Inicializar almacén con persistencia simple
    data_store_init("data_store.log");

    // Limpiar almacén y logger
    data_store_cleanup();

    return 0;
}
