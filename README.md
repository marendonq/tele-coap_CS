## Servidor CoAP (Thread-per-Request), Simulador ESP y Cliente CLI

Proyecto en C (Linux) que implementa un servidor CoAP sobre UDP, un simulador multi‑hilo (ESP32) y un cliente CLI. No usa librerías externas de CoAP; incluye parser/serializador propio y persistencia simple.

### Componentes
- `server/`
  - `main.c`: entrada; inicia logger y persistencia.
  - `server_thread_per_request.c`: socket UDP + threading por mensaje.
  - `coap_parser.{h,c}`: parseo/serialización CoAP (adaptador a `CoapMessage`). Deriva `uri_path` y `content_format`.
  - `coap_router.{h,c}`: ruteo por `Uri-Path` y manejo de métodos GET/POST/PUT/DELETE.
  - `data_store.{h,c}`: persistencia simple (último JSON por recurso), con respaldo en `data_store.log`.
  - `message.{h,c}`: modelo y rutinas CoAP compartidas (cliente/servidor).
  - `logger.{h,c}`: logging a `server.log`.
- `esp/`
  - `multi_client_threaded.c`: simulador multi‑cliente (hilos) que envía CoAP únicamente con método POST usando `message.{h,c}`.
- `client/`
  - `coap_cli.c`: cliente de línea de comandos para probar los 4 métodos.

---

## Requisitos
- Linux o WSL.
- Paquetes: `build-essential`, `gcc`.

```bash
sudo apt update
sudo apt install -y build-essential gcc
```

## Compilación

Servidor:
```bash
cd "esp y server/server"
gcc -o coap_server main.c server_thread_per_request.c coap_parser.c coap_router.c data_store.c message.c logger.c -lpthread
```

Simulador ESP (solo POST):
```bash
cd "../esp"
gcc -o multi_client multi_client_threaded.c ../server/message.c -lpthread
```

Cliente CLI:
```bash
cd "../client"
gcc -o coap_cli coap_cli.c ../server/message.c
```

## Ejecución

Servidor (terminal 1):
```bash
cd "esp y server/server"
./coap_server 5683 server.log
```

Simulador (terminal 2, ejemplos):
```bash
cd "../esp"
# POST 2 dispositivos, 2 rondas
./multi_client 127.0.0.1 5683 /sensors/temp 2 500 2 con post
# (El simulador solo envía POST; para GET/PUT/DELETE usa el cliente CLI)
```

Cliente CLI (terminal 3, ejemplos):
```bash
cd "esp y server/client"
./coap_cli 127.0.0.1 5683 /sensors/temp post con '{"temp_c":21.0}'
./coap_cli 127.0.0.1 5683 /sensors/temp get con
./coap_cli 127.0.0.1 5683 /sensors/temp put con '{"temp_c":23.4}'
./coap_cli 127.0.0.1 5683 /sensors/temp delete con
```

## Comportamiento esperado
- Métodos soportados (RFC 7252 básicos):
  - GET → 2.05 Content (69) con payload JSON del último valor guardado.
  - POST/PUT → 2.04 Changed (68). Guardan el JSON recibido para ese `Uri-Path`.
  - DELETE → 2.02 Deleted (66). Elimina el recurso del almacén (re‑escribe `data_store.log`).
- Persistencia: `data_store.log` acumula entradas; el almacén en memoria conserva solo el último valor por recurso. Tras DELETE, se borra del archivo.

## Diagnóstico
```bash
tail -f server.log           # ver eventos del servidor
cat server/data_store.log    # ver respaldos de valores por recurso
```

## Conexion con EC2

```
ssh -i tu_clave.pem ubuntu@<IP_PUBLICA_DE_TU_EC2>

```