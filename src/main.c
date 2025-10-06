#include "config.h"
#include "routes.h"
#include "persistence.h"
#include "data_store.h"
#include "coap_api.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("MAIN: Iniciando servidor...\n");
    AppConfig cfg;
    set_default_config(&cfg);
    parse_config(&cfg, argc, argv);
    printf("MAIN: Configuración cargada - Puerto: %d, Log: %s, Store: %s\n", 
           cfg.port, cfg.log_file, cfg.store_file);

    printf("MAIN: Inicializando persistencia...\n");
    if (init_persistence(cfg.store_file) != 0)
    {
        printf("MAIN: ERROR - Falló la inicialización de persistencia\n");
        return 1;
    }
    printf("MAIN: Persistencia inicializada correctamente\n");
    
    printf("MAIN: Registrando rutas...\n");
    if (register_routes(cfg.resource_path) != 0)
    {
        printf("MAIN: ERROR - Falló el registro de rutas\n");
        data_store_cleanup();
        return 1;
    }
    printf("MAIN: Rutas registradas correctamente\n");

    printf("MAIN: Iniciando servidor CoAP...\n");
    int rc = coap_server_start(cfg.port, cfg.log_file);
    printf("MAIN: Servidor terminó con código: %d\n", rc);

    data_store_cleanup();
    return rc;
}


