## Servidor CoAP – Documentación Completa (estructura src/)

### Tabla de contenidos

- [Introducción](#introducción)
- [Alcance (Scope)](#alcance-scope)
- [Requisitos del sistema (System requirements)](#requisitos-del-sistema-system-requirements)
- [Sistemas Operativos Compatibles](#sistemas-operativos-compatibles-compatible-operating-systems)
- [Lenguaje de programación](#lenguaje-de-programación-programming-language-used)
- [Dependencias](#dependencias-dependencies)
- [Herramientas de build](#herramientas-de-build-build-tools)
- [Estructura del proyecto](#estructura-del-proyecto)
- [Compilación](#compilación-compilation)
  - [Compilación con make (servidor)](#compilación-con-make-servidor)
  - [Compilar clientes (coap_cli, multi_client, esp_client)](#compilar-clientes-coap_cli-multi_client-esp_client)
- [Ejecución del servidor](#ejecución-del-servidor-running-the-server)
  - [Configuración (parámetros y defaults)](#configuración-parámetros-y-defaults)
  - [Verificación rápida](#verificación-rápida)
- [Probar con clientes](#probar-con-clientes)
  - [Cliente simple (métodos GET/POST/PUT/DELETE)](#cliente-simple-métodos-getpostputdelete)
  - [Cliente multi‑hilo (carga)](#cliente-multi‑hilo-carga)
  - [Simulador ESP (N dispositivos)](#simulador-esp-n-dispositivos)
- [Responsabilidades](#responsabilidades)
- [Manejo de conexiones](#manejo-de-conexiones)
- [Despacho de requests](#despacho-de-requests)
  - [Flujo detallado (End‑to‑End)](#flujo-detallado-end‑to‑end)
- [Logging](#logging)
- [Persistencia](#persistencia)
- [Principios de diseño](#principios-de-diseño)
- [Manejo de errores](#manejo-de-errores)
- [Rendimiento y escalabilidad](#rendimiento-y-escalabilidad)
- [Seguridad](#seguridad)
- [Limitaciones](#limitaciones)
- [Mejoras futuras](#mejoras-futuras)
- [Troubleshooting](#troubleshooting)
- [Referencia de API pública](#referencia-de-api-pública)

### Introducción

Este servidor implementa un servicio CoAP (Constrained Application Protocol) sobre UDP, orientado a escenarios IoT. La arquitectura es thread‑per‑request: cada datagrama se procesa en un hilo independiente, simplificando el manejo de errores por mensaje y el aislamiento entre requests.

El servidor separa la capa de protocolo de la lógica de negocio:
- Capa de protocolo (`src/networking/protocol/`): parseo y serialización CoAP, ruteo por URI y método, utilidades (logger, message) y almacenamiento simple.
- Capa de aplicación (`src/app/`): registro de rutas y handlers, configuración y persistencia.

Este documento describe la arquitectura, la forma de compilar/ejecutar (con la nueva estructura `src/`), y cómo probar los endpoints.

### Alcance (Scope)

El servidor está diseñado para:
- Manejar datagramas UDP y mensajes CoAP (CON/NON).
- Enrutar requests por `uri_path` y método hacia handlers definidos por la aplicación.
- Responder con semántica CoAP por defecto: ACK piggyback a CON y sin respuesta a NON.
- Registrar eventos y persistir datos de ejemplo asociados a `/sensors/temp`.

No cubre: Observe/Blockwise, DTLS, autenticación/autoritzación o almacenamiento transaccional.

### Requisitos del sistema (System requirements)

- Linux/Unix con POSIX.
- `gcc`, `make`.
- Librerías del sistema: pthreads, BSD sockets (`sys/socket.h`, `netinet/in.h`, `arpa/inet.h`).
- Permisos para abrir puerto UDP (por defecto 5683) y escribir en el directorio (logs, data store).

### Sistemas Operativos Compatibles (Compatible Operating Systems)

- GNU/Linux (probado).
- Otros Unix‑like con soporte pthreads y sockets BSD (ajustes menores podrían ser necesarios).

### Lenguaje de programación (Programming Language Used)

- C (C99/C11 compatible).

### Dependencias (Dependencies)

- Librerías estándar y del sistema: `pthread`, `socket`/BSD sockets, `stdio`, `stdlib`, `string`, `unistd`.
- Sin frameworks externos.

### Herramientas de build (Build Tools)

- `make` (Makefile en `src/`).
- Compilador: `gcc` (flags: `-pthread`, `-Wall`).

### Estructura del proyecto

```
src/
├── app/                        # Lógica de aplicación
│   ├── config.c
│   ├── handlers.c
│   ├── persistence.c
│   └── routes.c
├── networking/
│   ├── server.c                # UDP + thread-per-request
│   └── protocol/
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
├── Makefile                    # genera bin/servidor1_app
└── .gitignore

clients/
├── coap_cli/{coap_cli.c, message.c}
└── multi_client/{multi_client_threaded.c, message.c}

esp_client/
└── multi_client_threaded.c     # simulador de múltiples ESP (POST)
```

### Compilación (Compilation)

#### Compilación con make (servidor)

```bash
cd src
make
```
Resultado: `src/bin/servidor1_app`.

#### Compilar clientes (coap_cli, multi_client, esp_client)

Desde la raíz del repositorio:
```bash
# Cliente simple (coap_cli)
gcc -I src/utils/headers \
  clients/coap_cli/coap_cli.c clients/coap_cli/message.c \
  -o clients/coap_cli/coap_cli

# Cliente multi-hilo
gcc -pthread -I src/utils/headers \
  clients/multi_client/multi_client_threaded.c clients/multi_client/message.c \
  -o clients/multi_client/multi_client

# Simulador ESP (usa funciones de message.c)
gcc -pthread -I src/utils/headers \
  esp_client/multi_client_threaded.c src/networking/protocol/message.c \
  -o esp_client/esp_multi
```

### Ejecución del servidor (Running the Server)

```bash
./bin/servidor1_app              # puerto 5683, logs server.log por defecto
# o
./bin/servidor1_app 5683 server.log
```

#### Configuración (parámetros y defaults)

`src/app/config.c` establece por defecto:
- puerto: `5683`
- log: `server.log`
- store: `data_store.log`
- recurso base: `/sensors/temp`

Línea de comandos: `argv[1]=puerto`, `argv[2]=log_file`.

#### Verificación rápida

```bash
ss -ulpn | grep 5683 || sudo netstat -ulpn | grep 5683
tail -f server.log
```

### Probar con clientes

#### Cliente simple (métodos GET/POST/PUT/DELETE)

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

#### Cliente multi‑hilo (carga)

```bash
./clients/multi_client/multi_client 127.0.0.1 5683 /sensors/temp 20
```

#### Simulador ESP (N dispositivos)

```bash
./esp_client/esp_multi 127.0.0.1 5683 /sensors/temp 10 500 0 con
# host port path num_devices interval_ms rounds mode(non|con)
```

### Responsabilidades

- Configuración y persistencia: `app/config.c`, `app/persistence.c`.
- Rutas y handlers: `app/routes.c`, `app/handlers.c`.
- Servidor UDP: `networking/server.c`.
- Protocolo CoAP: `networking/protocol/*` (API, parser, router, message, logger, data_store).

### Manejo de conexiones

- Socket UDP + `bind()` al puerto.
- Bucle `recvfrom()`; se crea un hilo por datagrama (`pthread_create` + `pthread_detach`).
- Cada hilo parsea, enruta y responde (si corresponde), y libera recursos.

### Despacho de requests

- `coap_parser`: convierte bytes a `coap_message_t`, deriva `uri_path` y `content_format`.
- `coap_router`: llama al handler registrado para `(uri_path, método)`.
  - Mensaje CON → ACK piggyback con código de éxito por defecto.
  - Mensaje NON → `COAP_RESPONSE_NO_REPLY` (no se responde).

#### Flujo detallado (End‑to‑End)

1. `recvfrom()` recibe datagrama.
2. Hilo procesa: `parse_coap_message()` → `coap_router_handle_request()`.
3. Handler de aplicación crea/responde con payload; se obtiene `resp_code`.
4. `create_coap_response()` + `serialize_coap_message()` → `sendto()` al cliente.

### Logging

- `logger.c` añade timestamp y mensajes a `server.log`: inicio, parseo, envío, errores.

### Persistencia

- `data_store.c` mantiene un mapa en memoria y respaldo a `data_store.log` (append). `DELETE` reescribe el archivo para reflejar el estado.

### Principios de diseño

- Separación de capas (protocolo vs aplicación).
- Responsabilidad única por módulo.
- API clara para ruteo y handlers (`coap_register_handler`).

### Manejo de errores

- 4.04 si no existe handler para la ruta/método.
- 5.00 si el handler reporta error.
- Validaciones en parser/serializer (tamaños mínimos, TKL <= 8, etc.).

### Rendimiento y escalabilidad

- Thread‑per‑request: simple y efectivo para cargas moderadas.
- Recomendación para alta carga: pool de threads o modelo event‑driven (epoll), límites de concurrencia y back‑pressure.

### Seguridad

- Actualmente sin DTLS (CoAPs), autenticación ni autorización.
- Futuro: DTLS, tokens, autorización por recurso.

### Limitaciones

- Parser de opciones simplificado (delta básico); faltan Observe/Blockwise y retransmisiones avanzadas.
- Data store no transaccional.

### Mejoras futuras

- Pool de threads y límites de concurrencia.
- Parser/serializador de opciones extendido; soporte de más opciones CoAP.
- DTLS/seguridad, autenticación/autorización.
- Métricas/observabilidad (Prometheus), latencias y contadores por ruta.
- Backend persistente robusto (SQLite/LMDB), snapshots y recuperación.

### Troubleshooting

- No hay respuesta: si usas “non” (NON), no habrá respuesta; usa “con” (CON) para esperar ACK.
- Puerto ocupado: `ss -ulpn | grep 5683`.
- Firewall: `sudo ufw status`.
- Revisa `server.log` para parseo y errores de red.

### Referencia de API pública

Headers en `src/utils/headers/`:
- `coap_api.h`: `coap_register_handler(uri, method, fn)`, `coap_server_start(port, log)`.
- `coap_parser.h`: `parse_coap_message`, `create_coap_response`, `serialize_coap_message` y constantes (`COAP_METHOD_*`, `COAP_RESPONSE_*`, `COAP_TYPE_*`).
- `coap_router.h`: `coap_router_handle_request`.
- `server.h`: `start_server` y estructuras asociadas.

### Tabla de endpoints (por defecto)

| Recurso             | Método | Semántica                         | Respuesta OK | Payload de respuesta |
|---------------------|--------|-----------------------------------|--------------|----------------------|
| `/sensors/temp`     | POST   | Guarda payload JSON               | 2.01 Created | Texto confirmación   |
| `/sensors/temp`     | GET    | Devuelve último payload guardado  | 2.05 Content | JSON almacenado      |
| `/sensors/temp`     | PUT    | Actualiza payload JSON            | 2.04 Changed | Texto confirmación   |
| `/sensors/temp`     | DELETE | Elimina recurso                   | 2.02 Deleted | Texto confirmación   |

Notas:
- Mensajes NON (no confirmables) reciben `COAP_RESPONSE_NO_REPLY` (no se envía datagrama de respuesta).
- Content-Format esperado para POST/PUT: `application/json` (valor 50, opcional a nivel servidor ejemplo).

### Guía de compilación avanzada

- Variables de entorno útiles:
  - `CFLAGS_EXTRA="-O3 -g" make` para añadir flags adicionales.
  - `PREFIX` si deseas instalar binarios en otra ruta (no implementado por defecto, sugerido para futuro).
- Limpieza total:
```bash
cd src && make clean && rm -rf bin/
```
- Compilación con sanitizadores (debug):
```bash
cd src
make CFLAGS="-O0 -g -Wall -pthread -fsanitize=address,undefined -Iutils/headers"
```

### Pruebas End-to-End (E2E)

1) Arrancar servidor:
```bash
cd src && make && ./bin/servidor1_app 5683 server.log
```
2) Probar POST y GET (terminal 2):
```bash
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp post con '{"t":25.0}'
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp get con
```
3) Verificar `data_store.log` actualiza la entrada de `/sensors/temp`:
```bash
tail -n 5 src/data_store.log
```
4) Probar DELETE y GET (debe responder sin datos):
```bash
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp delete con
./clients/coap_cli/coap_cli 127.0.0.1 5683 /sensors/temp get con
```
5) Carga concurrente:
```bash
./clients/multi_client/multi_client 127.0.0.1 5683 /sensors/temp 50
```

### Métricas y observabilidad (sugerido)

- Contadores:
  - `total_messages_processed` (existe a nivel servidor en variables atómicas).
  - Por método (GET/POST/PUT/DELETE) y por código (2.xx, 4.xx, 5.xx) – agregar contadores en router/handlers.
- Temporizadores:
  - Latencia por `process_message` (timestamp antes/después), percentiles.
- Exportación:
  - Endpoint HTTP Prometheus (futuro), o logging con prefijo para scrap con vector/telegraf.

### Despliegue (sugerencias)

- Systemd unit (ejemplo mínimo):
```
[Unit]
Description=Servidor CoAP
After=network.target

[Service]
Type=simple
WorkingDirectory=/ruta/al/proyecto/src
ExecStart=/ruta/al/proyecto/src/bin/servidor1_app 5683 /ruta/al/proyecto/src/server.log
Restart=on-failure

[Install]
WantedBy=multi-user.target
```
- Docker (sugerencia base):
```
FROM debian:stable-slim
RUN apt-get update && apt-get install -y build-essential && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY src/ /app/src/
RUN make -C src
EXPOSE 5683/udp
CMD ["/app/src/bin/servidor1_app", "5683", "/app/src/server.log"]
```

### Guía de testing

- Unit tests (por implementar):
  - `coap_parser.c`: casos de TKL, opciones, payload marker 0xFF.
  - `coap_router.c`: rutas existentes vs inexistentes; NON vs CON.
  - `data_store.c`: set/get/delete con concurrencia (mutex activo).
- Integración:
  - Comunicaciones con sockets reales en loopback.
  - Validar persistencia en `data_store.log` y logging en `server.log`.

### Guía de seguridad (hardening mínimo)

- Validar límites de tamaño:
  - Buffer UDP (máx 1024 bytes), cortar payload excesivo.
  - TKL <= 8 (ya aplicado).
- Aislar permisos de archivo:
  - Ejecutar con usuario sin privilegios.
  - Directorio de trabajo con permisos restrictivos.
- Redes:
  - Firewall restringiendo puerto UDP 5683 solo a redes esperadas.
  - Considerar DTLS (futuro) para confidencialidad/integridad.

### FAQ

Q: ¿Por qué no recibo respuesta a veces?
A: Si envías NON, el servidor no responde (por diseño). Usa CON para esperar ACK.

Q: ¿Dónde se almacenan mis datos?
A: En memoria y en `src/data_store.log` (clave: `uri_path`, valor: JSON). DELETE reescribe el archivo.

Q: ¿Cómo cambio la ruta del recurso?
A: Modifica `resource_path` en `app/config.c` o parametriza con línea de comando al registrar rutas (a futuro).

Q: ¿Puedo agregar nuevos endpoints?
A: Sí, registra nuevos handlers con `coap_register_handler("/mi/ruta", COAP_METHOD_GET, miHandler)`. Añade la lógica en `app/handlers.c` y el registro en `app/routes.c`.

### Glosario

- CoAP: Protocolo ligero para IoT sobre UDP.
- CON/NON: Confirmable / Non-confirmable.
- ACK piggyback: Respuesta en el mismo datagrama de ACK.
- Uri-Path: Opción CoAP que compone la ruta (segmentos concatenados con '/').


