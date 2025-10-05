#include "config.h"
#include "routes.h"
#include "persistence.h"
#include "data_store.h"
#include "coap_api.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    AppConfig cfg;
    set_default_config(&cfg);
    parse_config(&cfg, argc, argv);

    if (init_persistence(cfg.store_file) != 0)
    {
        return 1;
    }
    if (register_routes(cfg.resource_path) != 0)
    {
        data_store_cleanup();
        return 1;
    }

    int rc = coap_server_start(cfg.port, cfg.log_file);

    data_store_cleanup();
    return rc;
}


