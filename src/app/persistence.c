#include "persistence.h"
#include "data_store.h"
#include <stdio.h>

int init_persistence(const char *store_file)
{
    if (data_store_init(store_file) != 0)
    {
        fprintf(stderr, "No se pudo inicializar data_store (%s)\n", store_file);
        return -1;
    }
    return 0;
}


