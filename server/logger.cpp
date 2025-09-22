#include "logger.h"
#include <iostream>
#include <ctime>

Logger::Logger(const std::string &filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "No se pudo abrir el archivo de log: " << filename << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(const std::string &message) {
    if (logFile.is_open()) {
        // Obtener timestamp
        std::time_t now = std::time(nullptr);
        logFile << std::ctime(&now) << ": " << message << std::endl;
    }
}
