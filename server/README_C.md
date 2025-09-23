# Servidor CoAP en C (Convertido desde C++)

Este proyecto implementa un servidor CoAP mínimo sobre UDP usando sockets de Berkeley y un cliente de carga en C basado en libcoap que simula múltiples dispositivos (ESP32) enviando datos de temperatura por POST.

## Conversión de C++ a C

El proyecto ha sido completamente convertido de C++ a C, manteniendo toda la funcionalidad original:

### Cambios realizados:
- **Logger**: Convertido de clase C++ a estructura C con funciones de inicialización/limpieza
- **Threading**: Reemplazado `std::thread` por `pthread`
- **Sincronización**: Reemplazado `std::mutex` y `std::atomic` por `pthread_mutex_t` y `atomic_int`
- **Strings**: Reemplazado `std::string` por arrays de char con `snprintf()`
- **Referencias**: Convertidas a punteros
- **Headers**: Actualizados para compatibilidad con C

### Archivos convertidos:
- `logger.cpp` → `logger.c`
- `main.cpp` → `main.c`
- `server.cpp` → `server.c`
- `server_thread_per_request.cpp` → `server_thread_per_request.c`
- `server.h` → actualizado para C
- `coap_parser.c/.h` → ya estaban en C

## Estructura del proyecto

- `main.c`: entrada del servidor
- `server.c`: servidor básico UDP (sin threading)
- `server_thread_per_request.c`: servidor con threading por request
- `server.h`: declaraciones de funciones
- `coap_parser.c/.h`: parser y helpers CoAP
- `logger.c/.h`: logging a archivo
- `esp/multi_client_threaded.c`: cliente de carga multi-thread con libcoap

## Requisitos

- **Linux/WSL**: Recomendado para compilar/ejecutar
- **Windows**: Usar WSL2 para compilación
- **Paquetes**: `build-essential`, `gcc`. Para el cliente: `libcoap-3-dev` y `libcoap-3-gnutls-dev`

## Compilación

### En Linux/WSL:
```bash
# Instalar dependencias
sudo apt update
sudo apt install -y build-essential gcc

# Compilar servidor básico
make coap_server

# Compilar servidor con threading
make coap_server_tpr

# Compilar cliente (desde esp/)
cd ../esp
sudo apt install -y libcoap-3-dev libcoap-3-gnutls-dev
gcc multi_client_threaded.c -o coap_multi_client_threaded \
  $(pkg-config --cflags --libs libcoap-3-gnutls) -lpthread
```

### En Windows (usando WSL):
```bash
# Abrir WSL
wsl

# Navegar al proyecto
cd /mnt/c/Almacen/Cosas\ de\ la\ U/2025-2/Internet\ arquitectura\ y\ protocolos/Primer\ Proyecto/esp\ y\ server/server

# Compilar
make all
```

## Ejecución

### Servidor básico:
```bash
./coap_server 5683 server.log
```

### Servidor con threading:
```bash
./coap_server_tpr 5683 server.log
```

### Cliente de prueba:
```bash
cd ../esp
./coap_multi_client_threaded 127.0.0.1 5683 /sensors/temp 5 2000 10 non
```

## Parámetros

- `puerto` (default 5683)
- `archivo_log` (opcional, default `server.log`)

## Funcionalidad

El servidor:
- Escucha en puerto UDP especificado
- Parsea mensajes CoAP
- Responde a POST en `/sensors/temp`
- Envía confirmación JSON
- Registra eventos en archivo de log

El cliente:
- Simula múltiples dispositivos ESP32
- Envía datos de temperatura en JSON
- Soporta mensajes CON y NON
- Configurable por intervalos y rondas

## Verificación

```bash
# Ver puerto abierto
ss -lun | grep 5683

# Seguir logs
tail -f server.log

# Capturar tráfico UDP CoAP
sudo tcpdump -i any udp port 5683 -vv -X
```

## Limpieza

```bash
make clean
cd ../esp && rm -f coap_multi_client_threaded
```

## Notas importantes

- El código ahora es 100% C estándar (C99)
- Mantiene toda la funcionalidad original
- Compatible con sistemas Unix/Linux
- Requiere WSL en Windows para compilación
- Threading implementado con pthread
- Gestión de memoria manual (malloc/free)
