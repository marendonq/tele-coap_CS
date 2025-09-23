#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct {
    FILE *logFile;
} Logger;

// Funciones del logger
Logger* logger_init(const char *filename);
void logger_cleanup(Logger *logger);
void logger_log(Logger *logger, const char *message);

#ifdef __cplusplus
}
#endif

#endif

