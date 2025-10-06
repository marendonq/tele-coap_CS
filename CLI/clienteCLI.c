#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../src/utils/headers/coap_api.h"

#define INPUT_SIZE 512

void print_help(void) {
    printf("\n=== CoAP CLI Client Interactivo ===\n");
    printf("Comandos disponibles:\n");
    printf("  get <host> <port> <path> <con|non>\n");
    printf("  post <host> <port> <path> <con|non> <json>\n");
    printf("  put <host> <port> <path> <con|non> <json>\n");
    printf("  delete <host> <port> <path> <con|non>\n");
    printf("  help - Mostrar esta ayuda\n");
    printf("  quit/exit - Salir del programa\n\n");
    
    printf("Ejemplos:\n");
    printf("  get 127.0.0.1 5683 /sensors/temp con\n");
    printf("  post 127.0.0.1 5683 /sensors/temp con '{\"value\":25.5}'\n");
    printf("  put 127.0.0.1 5683 /sensors/temp non '{\"value\":30.0}'\n");
    printf("  delete 127.0.0.1 5683 /sensors/temp con\n\n");
}

int validate_method(const char *method) {
    if (strcmp(method, "get") == 0 || strcmp(method, "GET") == 0) return 1;
    if (strcmp(method, "post") == 0 || strcmp(method, "POST") == 0) return 1;
    if (strcmp(method, "put") == 0 || strcmp(method, "PUT") == 0) return 1;
    if (strcmp(method, "delete") == 0 || strcmp(method, "DELETE") == 0) return 1;
    return 0;
}

int validate_mode(const char *mode) {
    if (strcmp(mode, "con") == 0 || strcmp(mode, "CON") == 0) return 1;
    if (strcmp(mode, "non") == 0 || strcmp(mode, "NON") == 0) return 1;
    return 0;
}

int validate_port(const char *port_str) {
    int port = atoi(port_str);
    if (port <= 0 || port > 65535) return 0;
    
    // Verificar que todos los caracteres sean dígitos
    for (int i = 0; port_str[i]; i++) {
        if (!isdigit(port_str[i])) return 0;
    }
    return 1;
}

int main(void) {
    char line[INPUT_SIZE];

    printf("=== CoAP CLI Client Interactivo ===\n");
    printf("Usa 'help' para ver comandos disponibles o 'quit' para salir.\n\n");

    while (1) {
        printf("coap> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        // quitar salto de línea
        line[strcspn(line, "\n")] = 0;

        // Comandos especiales
        if (strncmp(line, "quit", 4) == 0 || strncmp(line, "exit", 4) == 0) {
            break;
        }
        
        if (strncmp(line, "help", 4) == 0) {
            print_help();
            continue;
        }

        // separar tokens
        char *method = strtok(line, " ");
        char *host = strtok(NULL, " ");
        char *port_str = strtok(NULL, " ");
        char *path = strtok(NULL, " ");
        char *mode = strtok(NULL, " ");
        char *payload = strtok(NULL, "");

        if (!method || !host || !port_str || !path || !mode) {
            printf(" Uso inválido. Escribe 'help' para ver ejemplos.\n");
            continue;
        }

        // Validaciones
        if (!validate_method(method)) {
            printf(" Método inválido '%s'. Use: get, post, put, delete\n", method);
            continue;
        }
        
        if (!validate_port(port_str)) {
            printf(" Puerto inválido '%s'. Debe ser un número entre 1 y 65535.\n", port_str);
            continue;
        }
        
        if (!validate_mode(mode)) {
            printf(" Modo inválido '%s'. Use: con, non\n", mode);
            continue;
        }

        int port = atoi(port_str);

        printf("\n Enviando request CoAP:\n");
        printf("   Host: %s\n", host);
        printf("   Port: %d\n", port);
        printf("   Method: %s\n", method);
        printf("   Path: %s\n", path);
        printf("   Mode: %s\n", mode);
        if (payload && strlen(payload) > 0) {
            printf("   Payload: %s\n", payload);
        }
        printf("\n");

        // llamar a la API
        int r = coap_send_request(host, port, path, method, mode, payload);
        if (r == 0) {
            printf(" Request enviado exitosamente!\n");
        } else {
            printf(" Error al enviar request (código: %d)\n", r);
        }
        printf("\n");
    }

    printf(" Saliendo...\n");
    return 0;
}
