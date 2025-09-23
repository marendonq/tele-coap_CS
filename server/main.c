#include "server.h"
#include "logger.h"
#include <stdlib.h>   // atoi
#include <stdio.h>    // printf

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

    // Inicializar logger
    Logger *logger = logger_init(logFile);
    if (!logger) {
        fprintf(stderr, "Error al inicializar logger\n");
        return 1;
    }
    
    logger_log(logger, "Servidor iniciado...");

    // Iniciar servidor UDP
    start_server(port, logger);

    // Limpiar logger
    logger_cleanup(logger);

    return 0;
}
