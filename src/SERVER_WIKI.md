## Servidor CoAP – Documentación (estructura nueva en `src/`)

### Tabla de contenidos

- [Introducción](#introducción)
- [Alcance (Scope)](#alcance-scope)
- [Requisitos del sistema](#requisitos-del-sistema-system-requirements)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Dependencias y herramientas](#dependencias-y-herramientas)
- [Compilación](#compilación)
  - [Compilar servidor (src/)](#compilar-servidor-src)
  - [Compilar clientes (clients/ y esp_client/)](#compilar-clientes-clients-y-esp_client)
- [Ejecución](#ejecución)
  - [Ejecutar servidor](#ejecutar-servidor)
  - [Probar con clientes](#probar-con-clientes)
- [Responsabilidades del servidor](#responsabilidades-del-servidor)
- [Manejo de conexiones](#manejo-de-conexiones)
- [Despacho de requests](#despacho-de-requests)
- [Logging y persistencia](#logging-y-persistencia)
- [Principios de diseño](#principios-de-diseño)
- [Manejo de errores](#manejo-de-errores)
- [Rendimiento y seguridad](#rendimiento-y-seguridad)
- [Limitaciones y mejoras futuras](#limitaciones-y-mejoras-futuras)
- [Troubleshooting](#troubleshooting)

### Introducción

Este documento describe el servidor CoAP implementado en C sobre UDP, siguiendo una arquitectura modular con separación entre protocolo y aplicación. En la nueva estructura (`src/`), todo el código del servidor y la capa de protocolo está autocontenido para facilitar mantenimiento y despliegue. El servidor procesa cada datagrama en un hilo independiente (thread-per-request) y expone un endpoint de ejemplo `/sensors/temp` con handlers para GET/POST/PUT/DELETE.

### Alcance (Scope)

El servidor está diseñado para:
- Recibir datagramas UDP y parsear mensajes CoAP (CON/NON).
- Enrutar por `uri_path` y método (GET/POST/PUT/DELETE) hacia handlers de aplicación.
- Responder con ACK piggyback para CON u omitir respuesta para NON.
- Persistir en memoria con respaldo a archivo (append) y registrar eventos en `server.log`.

No implementa: Observe, Blockwise, DTLS, autenticación/autorización, ni almacenamiento transaccional.

### Requisitos del sistema (System requirements)

- Linux/Unix con POSIX, `gcc`, `make`, `pthread`, BSD sockets.
- Permisos para abrir el puerto UDP (por defecto 5683), permisos de escritura en el directorio para `server.log` y `data_store.log`.

### Estructura del proyecto

```
src/
├── app/                        # Lógica de aplicación (handlers/rutas/persistencia/config)
│   ├── config.c
│   ├── handlers.c
│   ├── persistence.c
│   └── routes.c
├── networking/
│   ├── server.c                # Servidor UDP (thread‑per‑request)
│   └── protocol/               # Capa CoAP autocontenida
│       ├── coap_api.c
│       ├── coap_parser.c
│       ├── coap_router.c
│       ├── data_store.c
│       ├── logger.c
│       └── message.c
├── utils/headers/              # Headers unificados
│   ├── coap_api.h  coap_parser.h  coap_router.h  message.h
│   ├── data_store.h  logger.h  server.h
│   ├── handlers.h  routes.h  persistence.h  config.h
├── main.c
├── Makefile                    # build del servidor → bin/servidor1_app
└── .gitignore

clients/                        # utilidades de prueba (fuera del core)
├── coap_cli/
│   ├── coap_cli.c
│   └── message.c
└── multi_client/
    ├── multi_client_threaded.c
    └── message.c

esp_client/                     # simulador de múltiples ESP (POST concurrentes)
└── multi_client_threaded.c
```

### Dependencias y herramientas

- Compilador C: `gcc` (C99/C11), `make`.
- Flags: `-pthread` para servidor y clientes multi-hilo.

### Compilación

#### Compilar servidor (src/)

```bash
cd src
make
```
Resultado: `src/bin/servidor1_app`.

#### Compilar clientes (clients/ y esp_client/)

Desde la raíz del repositorio:
```bash
# Cliente simple (coap_cli)
gcc -I src/utils/headers \
  clients/coap_cli/coap_cli.c clients/coap_cli/message.c \
  -o clients/coap_cli/coap_cli

# Cliente multi‑hilo
gcc -pthread -I src/utils/headers \
  clients/multi_client/multi_client_threaded.c clients/multi_client/message.c \
  -o clients/multi_client/multi_client

# Simulador ESP (requiere message.c del protocolo)
gcc -pthread -I src/utils/headers \
  esp_client/multi_client_threaded.c src/networking/protocol/message.c \
  -o esp_client/esp_multi
```

### Ejecución

#### Ejecutar servidor

```bash
./bin/servidor1_app              # usa 5683 y server.log por defecto
# o personalizados
./bin/servidor1_app 5683 server.log
```

Verificar servicio y logs:
```bash
ss -ulpn | grep 5683 || sudo netstat -ulpn | grep 5683
tail -f server.log
```

#### Probar con clientes

Cliente simple (métodos CoAP):
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

Prueba de carga:
```bash
./clients/multi_client/multi_client 127.0.0.1 5683 /sensors/temp 20
```

Simulación ESP (POST concurrentes):
```bash
./esp_client/esp_multi 127.0.0.1 5683 /sensors/temp 10 500 0 con
# host port path num_devices interval_ms rounds mode(non|con)
```

### Responsabilidades del servidor

- Inicializar configuración (`app/config.c`) y persistencia (`app/persistence.c`).
- Registrar rutas y handlers (`app/routes.c`, `app/handlers.c`).
- Levantar servidor UDP (`networking/server.c`).
- Parsear/serializar CoAP y rutear (`protocol/*`).

### Manejo de conexiones

- Socket UDP + `bind()` al puerto configurado.
- `recvfrom()` en bucle; por cada datagrama crea un hilo (`pthread_create`) → `process_message`.
- Hilo responde (si aplica) con `sendto()` y se detacha.

### Despacho de requests

Flujo: parseo CoAP → `coap_router_handle_request` → handler por `(uri_path,método)`.
- CON: responde con ACK piggyback y código de éxito por defecto.
- NON: marca `COAP_RESPONSE_NO_REPLY` (no se responde).

### Logging y persistencia

- `logger.c`: timestamp + mensaje a `server.log`.
- `data_store.c`: memoria + archivo `data_store.log` (append y reescritura en delete).

### Principios de diseño

- Separación de capas: app vs protocolo.
- Responsabilidad única por módulo.
- API de ruteo y handlers clara (`coap_register_handler`).

### Manejo de errores

- 4.04 si no hay handler; 5.00 si falla el handler.
- Validaciones de tamaños/valores en parser/serializer.
- Logs en fallos de red o parseo.

### Rendimiento y seguridad

- Thread-per-request: simple pero puede saturar bajo alta carga.
- Recomendado: pool de threads/límites de concurrencia, DTLS/seguridad futura.

### Limitaciones y mejoras futuras

- Falta Observe, Blockwise y retransmisiones avanzadas.
- Sin DTLS ni authz.
- Data store no transaccional.

### Troubleshooting

- No hay respuesta: usa “con” (CON) en el cliente; NON no recibe ACK.
- Puerto ocupado: `ss -ulpn | grep 5683`.
- Firewall: `sudo ufw status`.
- Logs: `tail -f server.log` para ver parseo y envíos.

## Servidor CoAP – Documentación

### Tabla de contenidos

- [Introducción](#introducción)
- [Alcance (Scope)](#alcance-scope)
- [Requisitos del sistema (System requirements)](#requisitos-del-sistema-system-requirements)
- [Sistemas Operativos Compatibles](#sistemas-operativos-compatibles-compatible-operating-systems)
- [Lenguaje de programación](#lenguaje-de-programación-programming-language-used)
- [Dependencias](#dependencias-dependencies)
- [Herramientas de build](#herramientas-de-build-build-tools)
- [Compilación](#compilación-compilation)
  - [Compilación con make](#compilación-con-make)
  - [Compilación manual sin make](#compilación-manual-sin-make)
- [Ejecución del servidor](#ejecución-del-servidor-running-the-server)
  - [Configuración](#configuración-configuration)
- [Responsabilidades](#responsabilidades-responsibilities)
- [Manejo de conexiones](#manejo-de-conexiones-connection-handling)
- [Despacho de requests](#despacho-de-requests-request-dispatching)
  - [Flujo detallado de un mensaje](#flujo-detallado-de-un-mensaje-end-to-end)
  - [Ejemplos de interacción](#ejemplos-de-interacción)
- [Gestión de usuarios y juego](#gestión-de-usuarios-y-juego-user-and-game-management)
- [Logging](#logging)
- [Principios de diseño](#principios-de-diseño-design-principles)
- [Manejo de errores](#manejo-de-errores-error-handling)
- [Referencia de API pública](#referencia-de-api-pública-api-reference)
- [Rendimiento y escalabilidad](#rendimiento-y-escalabilidad-performance--scalability)
- [Seguridad](#seguridad-security)
- [Limitaciones](#limitaciones)
- [Mejoras futuras](#mejoras-futuras-future-improvements)
- [Troubleshooting y verificación](#troubleshooting-y-verificación-troubleshooting)
- [Verificación rápida](#verificación-rápida-quick-check)
- [Métricas sugeridas](#métricas-sugeridas-observability)
- [Testing sugerido](#testing-sugerido)
- [Conclusión](#conclusión)

### Introducción

Este documento ofrece una visión integral del componente Servidor desarrollado en C que implementa un servicio CoAP (Constrained Application Protocol) sobre UDP, pensado para escenarios IoT y sistemas con recursos limitados. El servidor administra la recepción de datagramas, interpreta mensajes mediante CoAP, enruta solicitudes hacia handlers de negocio y emite respuestas conformes al protocolo.

El servidor fue construido como parte de un proyecto universitario, con objetivos de confiabilidad, modularidad y claridad. La implementación sigue prácticas de programación estructurada y separa responsabilidades en componentes bien definidos: networking/threads, parseo y serialización del protocolo, ruteo de solicitudes, logging y persistencia, además de la lógica de negocio (handlers) que utiliza dicha infraestructura. Esta división reduce el acoplamiento, facilita el mantenimiento y promueve la reutilización de la capa de protocolo en múltiples aplicaciones.

En términos de arquitectura, se adopta un modelo "thread-per-request" (un hilo por datagrama recibido), lo que simplifica el flujo de control y el aislamiento de errores por mensaje. A su vez, la capa de protocolo se desacopla de la aplicación: 

- Capa de protocolo (`protocolo/`): parseo/serialización CoAP, ruteo por URI y método, servidor UDP y logging.
- Capa de aplicación (`servidor1/`): definición de rutas y handlers específicos, configuración y persistencia.

Este documento sirve como referencia técnica de la arquitectura del servidor, sus funcionalidades, dependencias y papel dentro del ecosistema de la solución. Aunque asume familiaridad básica con C, se ha redactado para que sea accesible a desarrolladores que deseen entender, mantener o extender el sistema. Además, se señalan limitaciones actuales y líneas claras de evolución hacia escenarios de producción (pool de threads, DTLS/seguridad, parser extendido, métricas/observabilidad, y mejoras de persistencia).

### Alcance (Scope)

El servidor está diseñado para:

- Manejar múltiples mensajes de clientes concurrentes mediante un modelo thread-per-request (un hilo por datagrama UDP) para simplicidad y aislamiento.
- Interpretar y responder mensajes conforme a CoAP: parseo, validación básica y serialización de respuestas.
- Despachar solicitudes hacia handlers de negocio con base en `uri_path` y método (GET/POST/PUT/DELETE) usando la API de ruteo.
- Mantener estado de recursos en memoria con persistencia simple a archivo (append) para inspección y recuperación básica.
- Responder con semántica CoAP por defecto: ACK piggyback para mensajes CON y omitir respuesta para NON.
- Registrar actividad del servidor en un archivo de log para depuración y auditoría.
- Permitir configuración de puerto, archivo de log, archivo de store y ruta de recurso base.

El servidor no:

- Implementa características avanzadas del protocolo (Observe, Blockwise, negociación de contenido completa, retransmisiones/gestión de tiempo de confirmables a nivel de transporte).
- Provee seguridad avanzada (DTLS/CoAPs), autenticación o autorización.
- Garantiza persistencia transaccional ni almacenamiento robusto (usa un archivo de texto simple; no hay snapshots ni journaling).
- Ofrece interfaz gráfica.
- Gestiona usuarios, sesiones o lógica de juego (la lógica incluida es de ejemplo, centrada en recursos tipo sensor).

### Requisitos del sistema (System requirements)

Para compilar y ejecutar el servidor se requiere:

- **Sistema Operativo**: Linux u otro Unix-like con soporte POSIX.
- **Toolchain C**: `gcc` (o compatible) y `make` para compilación automatizada.
- **Librerías del sistema**: encabezados y librerías para `pthread` y BSD sockets (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`).
- **Permisos de red**: capacidad de abrir un socket UDP en el puerto configurado (por defecto 5683). Si el puerto < 1024 puede requerir privilegios elevados.
- **Almacenamiento**: permisos de lectura/escritura en el directorio para crear/actualizar `server.log` y `data_store.log`.

Recomendado para desarrollo:

- `build-essential` en Debian/Ubuntu: `sudo apt install build-essential`.
- Herramientas de diagnóstico de red: `iproute2` (`ss`), `netcat`, `tcpdump`/`wireshark`.

### Sistemas Operativos Compatibles (Compatible Operating Systems)

- GNU/Linux (probado).
- Otros Unix-like con soporte para `pthread` y BSD sockets pueden funcionar con cambios mínimos.

### Lenguaje de programación (Programming Language Used)

- C (C99/C11 compatible).

### Dependencias (Dependencies)

- Librerías estándar: `pthread`, `sys/socket.h`, `netinet/in.h`, `arpa/inet.h`, `stdio.h`, `stdlib.h`, `string.h`, `unistd.h`.
- No se usan frameworks externos; el protocolo CoAP está implementado en el repositorio.

Archivos principales:

- Servidor (aplicación): `servidor1/main.c`, `servidor1/config.c|.h`, `servidor1/routes.c|.h`, `servidor1/handlers.c|.h`, `servidor1/persistence.c|.h`.
- Protocolo: `protocolo/coap_api.c|.h`, `protocolo/server_thread_per_request.c`, `protocolo/coap_parser.c|.h`, `protocolo/coap_router.c|.h`, `protocolo/data_store.c|.h`, `protocolo/logger.c|.h`, `protocolo/message.c|.h`.

### Herramientas de build (Build Tools)

- `make` con `makefile` en `esp y server/`.
- Compilador: `gcc` con flag `-pthread`.

Objetivos relevantes del makefile:

- `servidor1_app`: aplicación del servidor.
- `coap_cli` y `multi_client`: clientes de prueba.

### Compilación (Compilation)

#### Compilación con make

Desde el directorio `esp y server/`:

```bash
make
```

Esto construye:

- `servidor1_app` (servidor CoAP)
- `coap_cli` (cliente de prueba)
- `multi_client` (cliente multi-hilo)

Limpieza de binarios y objetos:

```bash
make clean
```

Notas del `makefile`:

- Usa `CC=gcc` y `CFLAGS=-pthread`.
- Incluye fuentes de `servidor1/` y de `protocolo/` para enlazar una única app.
- Añade `-Iprotocolo` para incluir headers del protocolo.

#### Compilación manual sin make

Si no deseas usar `make`, puedes compilar manualmente. Posiciónate en `esp y server/` y ejecuta:

```bash
gcc -pthread -Iprotocolo \
  servidor1/main.c servidor1/config.c servidor1/handlers.c servidor1/routes.c servidor1/persistence.c \
  protocolo/coap_api.c protocolo/server_thread_per_request.c protocolo/coap_parser.c protocolo/coap_router.c \
  protocolo/data_store.c protocolo/message.c protocolo/logger.c \
  -o servidor1_app
```

Para los clientes de ejemplo:

```bash
gcc client/coap_cli.c client/message.c -o coap_cli
gcc -pthread esp/multi_client_threaded.c esp/message.c -o multi_client
```

Consejos:

- Si faltan headers, instala `build-essential` (Debian/Ubuntu) o el equivalente en tu distribución.
- Añade rutas de include/librerías según tu entorno si cambias la estructura.

### Ejecución del servidor (Running the Server)

Valores por defecto (puerto 5683, logs en `server.log`, store en `data_store.log`, recurso `/sensors/temp`):

```bash
./servidor1_app
```

Con puerto y archivo de log personalizados:

```bash
./servidor1_app 5684 mi_log.log
```

Al iniciar, el servidor escucha UDP y procesa mensajes CoAP en `start_server` (thread-per-request).

#### Configuración (Configuration)

Parámetros configurables desde `servidor1/config.c`:

- `port` (int): puerto UDP. Por defecto `5683`.
- `log_file` (const char*): ruta del archivo de logs. Por defecto `server.log`.
- `store_file` (const char*): ruta de persistencia. Por defecto `data_store.log`.
- `resource_path` (const char*): ruta base del recurso. Por defecto `/sensors/temp`.

Línea de comandos soportada (ver `parse_config`):

- `argv[1]`: puerto
- `argv[2]`: archivo de log

Ejemplos:

```bash
./servidor1_app                # 5683, server.log
./servidor1_app 5684           # 5684, server.log
./servidor1_app 5684 my.log    # 5684, my.log
```

### Responsabilidades (Responsibilities)

- Inicializar configuración y persistencia: `servidor1/main.c`, `servidor1/config.c`, `servidor1/persistence.c`.
- Registrar rutas y handlers de negocio: `servidor1/routes.c`, `servidor1/handlers.c`.
- Arrancar servidor UDP y orquestar el flujo CoAP: `protocolo/coap_api.c`, `protocolo/server_thread_per_request.c`.
- Parsear/serializar CoAP y rutear por URI/método: `protocolo/coap_parser.c`, `protocolo/coap_router.c`.
- Registrar y despachar handlers: API de registro en `protocolo/coap_api.c`.
- Persistencia: `protocolo/data_store.c` (memoria + archivo de texto).

### Manejo de conexiones (Connection Handling)

- Socket UDP (`socket(AF_INET, SOCK_DGRAM, 0)`), `bind` al puerto configurado.
- Bucle principal con `recvfrom` para recibir datagramas.
- Por cada datagrama, se crea un thread (`pthread_create`) que ejecuta `process_message` y se hace `pthread_detach`.
- Métricas básicas: `active_threads`, `total_messages_processed`.

Archivo clave: `protocolo/server_thread_per_request.c`.

### Despacho de requests (Request Dispatching)

Flujo general:

1. `parse_coap_message` convierte bytes UDP en `coap_message_t` y deriva `uri_path` y `content_format`.
2. `coap_router_handle_request` busca handler por `uri_path` y método (`request->code`).
3. Para mensajes CON: ejecuta handler y devuelve código de éxito por defecto según método.
4. Para mensajes NON: marca `COAP_RESPONSE_NO_REPLY` (no se responde).
5. `create_coap_response` y `serialize_coap_message` construyen y serializan la respuesta para `sendto`.

Registro de rutas y handlers desde la app: `servidor1/routes.c` usando `coap_register_handler`.

#### Flujo detallado de un mensaje (End-to-end)

1. `recvfrom` captura datagrama UDP.
2. Se crea `ThreadData` con `MessageData` y `Logger` y se lanza `pthread_create`.
3. `process_message` invoca `parse_coap_message` → genera `coap_message_t request` con `uri_path` derivado.
4. `coap_router_handle_request` resuelve el handler por `(uri_path, code)` y decide:
   - NON → `COAP_RESPONSE_NO_REPLY` (no se envía respuesta).
   - CON → ejecuta handler y fija `out_code` según éxito.
5. `create_coap_response` construye la respuesta CoAP (ACK piggyback) y `serialize_coap_message` la empaqueta.
6. `sendto` envía la respuesta al cliente; se liberan recursos (`free_coap_message`).

#### Ejemplos de interacción

Supongamos `resource_path = /sensors/temp`.

- POST (guardar medición): payload JSON `{"t": 23.4}`
  - Handler: `HandlerFunctionTempPost`
  - Respuesta: `2.01 Created` con texto "Payload recibido y guardado (... bytes)".

- GET (consultar última medición):
  - Handler: `HandlerFunctionTempGet`
  - Respuesta: `2.05 Content` con el JSON guardado o mensaje "No hay datos para /sensors/temp".

- PUT (actualizar medición): `HandlerFunctionTempPut` → `2.04 Changed`.

- DELETE (eliminar medición): `HandlerFunctionTempDelete` → `2.02 Deleted`.

### Gestión de usuarios y juego (User and Game Management)

No aplica. La lógica de negocio por defecto gestiona recursos tipo sensor:

- `HandlerFunctionTempPost/Put`: guarda/actualiza payload asociado a `uri_path`.
- `HandlerFunctionTempGet`: recupera payload si existe.
- `HandlerFunctionTempDelete`: elimina el recurso.

### Logging

- `protocolo/logger.c` gestiona el archivo de log.
- Se registran eventos: inicio de servidor, recepción y parseo de mensajes, envío de respuestas y errores de red.

### Principios de diseño (Design Principles)

- Separación de capas: protocolo CoAP en `protocolo/` y lógica de aplicación en `servidor1/`.
- Responsabilidad única por módulo: networking, parser, router, logging, data store y handlers están desacoplados.
- API explícita para registrar rutas/handlers: `coap_register_handler(uri, method, fn)`.
- Arquitectura thread-per-request para simplicidad y aislamiento de fallos por mensaje.

### Manejo de errores (Error Handling)

- Validación en parsing/serialización (tamaños mínimos, TKL ≤ 8).
- Sin handler para ruta/método → 4.04 (PAGE_NOT_FOUND).
- Error en handler → 5.00 (SERVER_ERROR).
- Logs en errores de `socket`, `bind`, `recvfrom`, `sendto` y parseo.
- Liberación de recursos en `free_coap_message` y al finalizar cada thread.

### Referencia de API pública (API Reference)

Funciones expuestas relevantes (ver headers en `protocolo/` y `servidor1/`):

- `int coap_register_handler(const char* uri, uint8_t method, coap_handler_fn fn);`
  - Registra un handler para una ruta/método. Devuelve `0` en éxito.

- `int coap_server_start(int port, const char *logFileName);`
  - Inicializa logger y despliega el servidor UDP.

- `int parse_coap_message(const uint8_t *data, size_t len, coap_message_t *msg);`
  - Parsea buffer de bytes a `coap_message_t` y deriva `uri_path`.

- `int coap_router_handle_request(const coap_message_t *request, uint8_t *out_code, char *out_payload, size_t out_payload_size);`
  - Resuelve handler y calcula código de respuesta.

- `int create_coap_response(const coap_message_t *request, coap_message_t *response, uint8_t code, const char *payload, size_t payload_len);`
  - Construye respuesta CoAP (ACK piggyback por defecto).

- `size_t serialize_coap_message(const coap_message_t *msg, uint8_t *buffer, size_t buffer_size);`
  - Serializa mensaje CoAP listo para `sendto`.

Tipos y constantes (ver `coap_parser.h`): `coap_message_t`, `COAP_METHOD_*`, `COAP_RESPONSE_*`, `COAP_TYPE_*`.

### Rendimiento y escalabilidad (Performance & Scalability)

- Modelo actual: thread-per-request. Ventaja: simplicidad y aislamiento. Riesgo: demasiados threads bajo alta carga.
- Recomendaciones:
  - Limitar concurrencia (semáforo/contador) o usar pool de threads.
  - Reusar buffers y evitar copias innecesarias en payloads grandes.
  - Considerar modelo basado en eventos (`epoll`) si la carga es muy alta.

### Seguridad (Security)

- Actualmente sin DTLS (CoAPs) ni autenticación/autorización.
- Recomendaciones:
  - Añadir DTLS en el transporte para integridad/confidencialidad.
  - Validar `Content-Format` y tamaño máximo de payload.
  - Sanitizar `uri_path` y rechazar rutas fuera de un prefijo permitido.

### Limitaciones

- Tabla de rutas fija (`MAX_ROUTES = 16`) y sin mutex (riesgo si el registro fuera concurrente).
- `data_store` simple: memoria + archivo de texto; delete reescribe todo, sin transacciones.
- Parser de opciones simplificado (delta básico; faltan extensiones 13/14/15 y opciones avanzadas).
- Sin límites de concurrencia; thread-per-request puede saturar bajo alta carga.
- Mínimo soporte de semántica CoAP avanzada (retransmisiones, respuestas separadas, Observe, etc.).
- Sin DTLS (CoAPs), autenticación ni autorización.

### Mejoras futuras (Future Improvements)

- Proteger tabla de rutas con mutex y soportar rutas dinámicas/wildcards.
- Pool de threads o modelo basado en eventos (epoll) con límites de concurrencia.
- Parser de opciones completo y soporte de más opciones (Observe, Accept, Max-Age, etc.).
- Semántica CoAP avanzada: CON con retransmisión, NON con respuestas separadas cuando aplique.
- Backend de persistencia robusto (SQLite/LMDB), snapshots y recuperación atómica.
- Métricas y telemetría (Prometheus): latencia, RPS, errores por ruta/método.
- Validación/negociación de `Content-Format` y esquemas de payload.
- Seguridad: DTLS, autenticación (tokens), autorización por recurso.
- Suite de tests unitarios e integración.

### Troubleshooting y verificación (Troubleshooting)

- El servidor no arranca: verificar que el puerto no esté en uso (`ss -ulpn | grep 5683`).
- No llegan respuestas:
  - Confirmar que el mensaje es CON (no NON) si esperas respuesta inmediata.
  - Revisar firewall (`ufw status`) y reachability entre cliente/servidor.
  - Chequear `server.log` para errores de `recvfrom`/`sendto` o parseo.
- Persistencia no refleja cambios: confirmar permisos de escritura en `data_store.log`.

### Verificación rápida (Quick check)

```bash
make && ./servidor1_app &            # iniciar servidor
echo "OK" && tail -n +1 -f server.log # monitorear logs
```

Con un cliente CoAP (externo) enviar POST/GET a `coap://<host>:5683/sensors/temp`.

### Métricas sugeridas (Observability)

- Contadores: requests totales, por método, 4xx/5xx, bytes in/out.
- Temporizadores: latencia por handler.
- Gauge: threads activos, cola de trabajo (si hay pool).

### Testing sugerido

- Unit tests: parser/serializer, router, handlers.
- Integración: flujo end-to-end con sockets UDP y payloads variados.
- Carga: pruebas de concurrencia para evaluar saturación de threads.

### Conclusión

El servidor ofrece una arquitectura modular y clara: la capa de protocolo CoAP es reusable y la capa de aplicación define rutas/handlers con persistencia simple. Es adecuado para aprendizaje y prototipos. Con mejoras en concurrencia, parser, persistencia y seguridad, puede evolucionar hacia escenarios más exigentes.


