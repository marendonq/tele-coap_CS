#include "server.h"
#include "logger.h"
#include "data_store.h"
#include "coap_api.h"
#include <stdlib.h>   // atoi
#include <stdio.h>    // printf

void guardarDatos(const coap_message_t *msg){
    //! Solo los imprimo

    printf("[HANDLER] GET /temperatura\n");
    printf("Token length: %d\n", msg->tkl);

}


int main(int argc, char *argv[]) {
    int port = 5683;           // Puerto por defecto
    const char *logFile = "server.log"; // Archivo log por defecto

    // Leer argumentos de línea de comandos
    if (argc > 1) {
        port = atoi(argv[1]); // Primer argumento = puerto
    }
    if (argc > 2) {
        logFile = argv[2]; // Segundo argumento = archivo de log
    }

    coap_register_handler("/sensors/temp", COAP_METHOD_GET, guardarDatos);

    coap_server_start(port, logFile);

    // Inicializar almacén con persistencia simple
    data_store_init("data_store.log");

    // Limpiar almacén y logger
    data_store_cleanup();

    return 0;
}
