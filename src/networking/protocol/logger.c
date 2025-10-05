#include "logger.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

Logger* logger_init(const char *filename) {
    Logger *logger = (Logger*)malloc(sizeof(Logger));
    if (!logger) {
        return NULL;
    }
    logger->logFile = fopen(filename, "a");
    if (!logger->logFile) {
        fprintf(stderr, "No se pudo abrir el archivo de log: %s\n", filename);
        free(logger);
        return NULL;
    }
    return logger;
}

void logger_cleanup(Logger *logger) {
    if (logger) {
        if (logger->logFile) {
            fclose(logger->logFile);
        }
        free(logger);
    }
}

void logger_log(Logger *logger, const char *message) {
    if (logger && logger->logFile) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        size_t len = strlen(time_str);
        if (len > 0 && time_str[len-1] == '\n') {
            time_str[len-1] = '\0';
        }
        fprintf(logger->logFile, "%s: %s\n", time_str, message);
        fflush(logger->logFile);
    }
}


