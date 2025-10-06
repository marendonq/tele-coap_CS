#include "config.h"
#include <stdlib.h>

void set_default_config(AppConfig *cfg)
{
    cfg->port = 5683;
    cfg->log_file = "server.log";
    cfg->store_file = "data_store.log";  // Ruta simple en el directorio actual
    cfg->resource_path = "/sensors/temp";
}

void parse_config(AppConfig *cfg, int argc, char **argv)
{
    if (argc > 1)
    {
        cfg->port = atoi(argv[1]);
    }
    if (argc > 2)
    {
        cfg->log_file = argv[2];
    }
}


