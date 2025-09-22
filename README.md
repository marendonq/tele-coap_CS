## Servidor CoAP (Thread-per-Request) y Cliente de Pruebas

Este proyecto implementa un servidor CoAP mínimo sobre UDP usando sockets de Berkeley y un cliente de carga en C basado en libcoap que simula múltiples dispositivos (ESP32) enviando datos de temperatura por POST.

### ¿Cómo funciona?

- **Servidor (`server_thread_per_request.cpp`)**: crea un socket UDP, recibe datagramas y, por cada mensaje, lanza un thread para procesarlo (modelo thread-per-request). Se intenta parsear el datagrama como CoAP con un parser propio (`coap_parser.c`). Si es un POST a `coap://<IP>:<PUERTO>/sensors/temp` con payload, el servidor responde con un ACK CoAP y payload JSON de confirmación.
- **Punto de entrada (`main.cpp`)**: lee `puerto` y `archivo_log` opcionales, inicializa `Logger` y arranca el servidor.
- **Parser CoAP (`coap_parser.c/.h`)**: parseo simplificado del encabezado, token, opciones básicas y payload; utilidades para crear y serializar respuestas CoAP.
- **Logger (`logger.cpp/.h`)**: append a `server.log` con timestamp para cada evento.
- **Cliente (`esp/multi_client_threaded.c`)**: usa libcoap para crear N sesiones cliente concurrentes (un thread por dispositivo) que envían POST en `/sensors/temp` con JSON `{"id":"esp32-i","seq":N,"temp_c":T}`. Permite mensajes CON o NON, intervalos, rondas, etc.

## Estructura relevante

- `server/main.cpp`: entrada del servidor.
- `server/server_thread_per_request.cpp`: lógica UDP + threading por request.
- `server/server.h`: declaración de `start_server`.
- `server/coap_parser.c/.h`: parser y helpers CoAP.
- `server/logger.cpp/.h`: logging a archivo.
- `esp/multi_client_threaded.c`: cliente de carga multi-thread con libcoap.

## Requisitos

- Linux o WSL (Windows Subsystem for Linux) recomendado para compilar/ejecutar.
- Paquetes: `build-essential`, `gcc`, `g++`. Para el cliente: `libcoap-3-dev` y `libcoap-3-gnutls-dev`.

## Compilación

Desde el directorio `server/`:

```bash
sudo apt update
sudo apt install -y build-essential gcc g++
g++ -std=c++11 -Wall -Wextra -O2 -pthread -o coap_server_tpr \
  main.cpp server_thread_per_request.cpp logger.cpp coap_parser.c
```

Cliente (desde `esp/`):

```bash
sudo apt install -y libcoap-3-dev libcoap-3-gnutls-dev
gcc multi_client_threaded.c -o coap_multi_client_threaded \
  $(pkg-config --cflags --libs libcoap-3-gnutls) -lpthread
```

## Ejecución

Servidor (en `server/`):

```bash
./coap_server_tpr 5683 server.log
```

Parámetros:

- `puerto` (default 5683)
- `archivo_log` (opcional, default `server.log`)

Cliente (en `esp/`, en otra terminal):

```bash
./coap_multi_client_threaded <host> <puerto> <ruta> <num_devices> [interval_ms=5000] [rounds=0] [mode=non|con]
```

Ejemplo local:

```bash
./coap_multi_client_threaded 127.0.0.1 5683 /sensors/temp 5 2000 10 non
```

## Qué esperar en consola

- El servidor muestra IP/puerto del cliente, bytes recibidos, método CoAP y payload; responde con `2.04 Changed` y JSON `{ "status":"ok" ... }`.
- El cliente imprime por dispositivo la ronda, secuencia y temperatura enviada. En modo `con`, verás gestión de ACKs por libcoap.

## Pruebas rápidas

- Cambia `num_devices` y `interval_ms` para generar más o menos carga.
- Prueba `mode=con` para mensajes confirmables: `./coap_multi_client_threaded 127.0.0.1 5683 /sensors/temp 10 200 100 con`.
- Usa otra máquina en la misma red: reemplaza `127.0.0.1` por la IP del servidor.

## Verificación y diagnóstico

```bash
# Ver puerto abierto
ss -lun | grep 5683

# Seguir logs
tail -f server.log

# Capturar tráfico UDP CoAP (opcional, requiere privilegios)
sudo tcpdump -i any udp port 5683 -vv -X
```

Si no recibes respuestas:

- Verifica firewall (UFW/Windows Defender) permita UDP 5683.
- Asegura que el `host` del cliente es alcanzable (`ping <ip>`).
- En WSL, prueba primero con `127.0.0.1`. Para acceso desde LAN, usa WSL2 con mapeo de puertos o ejecuta en Linux nativo.

## Limpieza

```bash
rm -f coap_server_tpr
rm -f server.log
cd ../esp && rm -f coap_multi_client_threaded
```
