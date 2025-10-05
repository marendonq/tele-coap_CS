## Servidor CoAP – Guía Rápida (estructura src/)

### Índice
- [Introducción](#introducción)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Requisitos y dependencias](#requisitos-y-dependencias)
- [Compilación](#compilación)
- [Ejecución y verificación](#ejecución-y-verificación)
- [Clientes de prueba](#clientes-de-prueba)
- [Arquitectura y responsabilidades](#arquitectura-y-responsabilidades)
- [Manejo de errores y logging](#manejo-de-errores-y-logging)
- [Limitaciones y mejoras](#limitaciones-y-mejoras)
- [Troubleshooting](#troubleshooting)

### Introducción
Servidor CoAP sobre UDP escrito en C con arquitectura thread‑per‑request. La capa de protocolo (parseo/serialización CoAP, ruteo) está desacoplada de la lógica de aplicación (handlers/rutas/persistencia). La nueva estructura `src/` contiene todo lo necesario para compilar y correr el servidor de forma autocontenida.

### Estructura del proyecto
```
src/
├── app/                        # Lógica de aplicación (handlers/rutas/persistencia/config)
├── networking/                 # Red
│   ├── server.c                # UDP + threads por request
│   └── protocol/               # Capa CoAP (API, parser, router, message, logger, store)
├── utils/headers/              # Headers unificados
├── main.c
├── Makefile                    # genera bin/servidor1_app
└── .gitignore

clients/                        # utilidades para pruebas
├── coap_cli/{coap_cli.c, message.c}
└── multi_client/{multi_client_threaded.c, message.c}

esp_client/
└── multi_client_threaded.c     # simulador de múltiples ESP (POST)
```

### Requisitos y dependencias
- SO: Linux/Unix (POSIX), gcc, make.
- Librerías: pthreads, BSD sockets.
- Archivos de salida: `server.log`, `data_store.log` en el directorio de ejecución.

### Compilación
- Servidor (desde `src/`):
```bash
cd src
make
```
Resultado: `bin/servidor1_app`.

- Clientes (desde la raíz del repo):
```bash
# Cliente simple
gcc -I src/utils/headers \
  clients/coap_cli/coap_cli.c clients/coap_cli/message.c \
  -o clients/coap_cli/coap_cli

# Cliente multi‑hilo
gcc -pthread -I src/utils/headers \
  clients/multi_client/multi_client_threaded.c clients/multi_client/message.c \
  -o clients/multi_client/multi_client

# Simulador ESP
gcc -pthread -I src/utils/headers \
  esp_client/multi_client_threaded.c src/networking/protocol/message.c \
  -o esp_client/esp_multi
```

### Ejecución y verificación
- Servidor:
```bash
./bin/servidor1_app          # puerto 5683 por defecto
# o
./bin/servidor1_app 5683 server.log
```
- Verificar servicio y logs:
```bash
ss -ulpn | grep 5683 || sudo netstat -ulpn | grep 5683
tail -f server.log
```

### Clientes de prueba
- Métodos (cliente simple):
```bash
# POST
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp post con '{"t":25.1}'
# GET
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp get con
# PUT
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp put con '{"t":26.3}'
# DELETE
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp delete con
```
- Carga concurrente:
```bash
./clients/multi_client/multi_client 127.0.0.1 5683 /sensors/temp 20
```
- Simulador ESP (POST concurrentes):
```bash
./esp_client/esp_multi 127.0.0.1 5683 /sensors/temp 10 500 0 con
# host port path num_devices interval_ms rounds mode(non|con)
```

### Arquitectura y responsabilidades
- `networking/server.c`: socket UDP + `recvfrom` en bucle; crea un thread por mensaje; responde con `sendto`.
- `protocol/*`: parsea (CoAP) → enruta (por `uri_path` y método) → genera respuesta (ACK piggyback para CON).
- `app/*`: registra rutas y handlers; `handlers.c` implementa POST/GET/PUT/DELETE sobre `/sensors/temp` respaldado por `data_store`.
- `data_store.c`: memoria + archivo (append) con reescritura en delete.

### Manejo de errores y logging
- 4.04 si no hay handler; 5.00 si el handler falla; sin respuesta para NON.
- `logger.c` escribe timestamp + mensaje en `server.log` (inicio, parseo, envío, errores).

### Limitaciones y mejoras
- Sin Observe, Blockwise, retransmisiones avanzadas.
- Sin DTLS / autenticación / autorización.
- Modelo thread‑per‑request: simple, pero puede saturar en alta concurrencia.
- Mejoras sugeridas: pool de threads, límites de concurrencia, DTLS, parser de opciones extendido, métricas (Prometheus), backend persistente (SQLite/LMDB).

### Troubleshooting
- No hay respuesta: usar “con” (CON) en el cliente; NON no recibe ACK.
- Puerto ocupado: `ss -ulpn | grep 5683`.
- Firewall: `sudo ufw status`.
- Ver logs: `tail -f server.log`.


