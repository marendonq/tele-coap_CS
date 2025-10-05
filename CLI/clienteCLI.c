#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coap_api.h"

#define INPUT_SIZE 512

int main(void) {
    char line[INPUT_SIZE];

    printf("Cliente CoAP interactivo\n");
    printf("Comandos:\n");
    printf("  get <host> <port> <path> <con|non>\n");
    printf("  post <host> <port> <path> <con|non> <json>\n");
    printf("  put <host> <port> <path> <con|non> <json>\n");
    printf("  delete <host> <port> <path> <con|non>\n");
    printf("  quit/exit\n\n");

    while (1) {
        printf("coap> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        // quitar salto de línea
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "quit", 4) == 0 || strncmp(line, "exit", 4) == 0) {
            break;
        }

        // separar tokens
        char *method = strtok(line, " ");
        char *host = strtok(NULL, " ");
        char *port_str = strtok(NULL, " ");
        char *path = strtok(NULL, " ");
        char *mode = strtok(NULL, " ");
        char *payload = strtok(NULL, "");

        if (!method || !host || !port_str || !path || !mode) {
            printf("Uso inválido. Escribe 'help' para ejemplos.\n");
            continue;
        }

        int port = atoi(port_str);

        // llamar a la API
        int r = coap_send_request(host, port, path, method, mode, payload);
        if (r != 0) {
            printf("Error al enviar request\n");
        }
    }

    printf("Saliendo...\n");
    return 0;
}
