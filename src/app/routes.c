#include "routes.h"
#include "handlers.h"
#include "coap_api.h"
#include <stdio.h>

int register_routes(const char *resource_path)
{
    if (coap_register_handler(resource_path, COAP_METHOD_POST, HandlerFunctionTempPost) != 0)
    {
        fprintf(stderr, "Error registrando handler POST %s\n", resource_path);
        return -1;
    }
    if (coap_register_handler(resource_path, COAP_METHOD_GET, HandlerFunctionTempGet) != 0)
    {
        fprintf(stderr, "Error registrando handler GET %s\n", resource_path);
        return -1;
    }
    if (coap_register_handler(resource_path, COAP_METHOD_PUT, HandlerFunctionTempPut) != 0)
    {
        fprintf(stderr, "Error registrando handler PUT %s\n", resource_path);
        return -1;
    }
    if (coap_register_handler(resource_path, COAP_METHOD_DELETE, HandlerFunctionTempDelete) != 0)
    {
        fprintf(stderr, "Error registrando handler DELETE %s\n", resource_path);
        return -1;
    }
    return 0;
}


