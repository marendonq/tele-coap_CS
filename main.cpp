#include "server.h"
#include "logger.h"
#include <cstdlib>   // atoi
#include <iostream>  // cout

int main(int argc, char *argv[]) {
    int port = 5683;           // Puerto por defecto
    std::string logFile = "server.log"; // Archivo log por defecto

    // Leer argumentos de línea de comandos
    if (argc > 1) {
        port = std::atoi(argv[1]); // Primer argumento = puerto
    }
    if (argc > 2) {
        logFile = argv[2]; // Segundo argumento = archivo de log
    }

    // Inicializar logger
    Logger logger(logFile);
    logger.log("Servidor iniciado...");

    // Iniciar servidor UDP
    start_server(port, logger);

    return 0;
}
