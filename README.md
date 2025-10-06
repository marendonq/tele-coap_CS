# CoAP-Server — Servidor CoAP

Un servidor CoAP (Constrained Application Protocol) escrito en C que maneja comunicación IoT con múltiples clientes usando el protocolo CoAP estándar.

Este servidor usa arquitectura thread-per-request para manejar conexiones concurrentes y implementa el protocolo CoAP completo para comunicación IoT.

## Características Principales

- **Soporte para múltiples clientes simultáneos**
- **Protocolo CoAP completo** (GET, POST, PUT, DELETE)
- **Arquitectura thread-per-request** para alta concurrencia
- **Persistencia de datos** con almacenamiento en archivo
- **Validación JSON** para integridad de datos
- **Sistema de logging** completo para debugging
- **Cliente CLI interactivo** para pruebas manuales
- **Simulador ESP32** para múltiples dispositivos IoT

## Requisitos

- **Sistema Unix** (Linux/macOS/WSL)
- **GCC** (compilador C)
- **Make** (para compilar el proyecto)
- **pthread** (librería de threads POSIX)

## Instrucciones de Compilación

### Compilar el Servidor
```bash
cd src
make
```

### Compilar el Cliente CLI
```bash
cd CLI
make
```

### Compilar el Simulador ESP32
```bash
cd esp_client
make
```

## Ejecutar el Servidor

Para iniciar el servidor:

```bash
./src/bin/servidor1_app <puerto> <archivo_log>
```

### Ejemplo
```bash
./src/bin/servidor1_app 5683 server.log
```

El servidor comenzará a escuchar conexiones de clientes en el puerto especificado. Toda la actividad se registrará en el archivo proporcionado.

## Usar los Clientes

### Cliente CLI Interactivo
```bash
./CLI/clienteCLI
```

**Comandos disponibles:**
```
coap> get 127.0.0.1 5683 /sensors/temp con
coap> post 127.0.0.1 5683 /sensors/temp con '{"value":25.5}'
coap> put 127.0.0.1 5683 /sensors/temp non '{"value":30.0}'
coap> delete 127.0.0.1 5683 /sensors/temp con
coap> help
coap> quit
```

### Simulador ESP32
```bash
./esp_client/esp_multi <host> <puerto> <ruta> <dispositivos> <intervalo> <rondas>
```

**Ejemplo:**
```bash
./esp_client/esp_multi 127.0.0.1 5683 /sensors/temp 5 1000 10
```

## Qué Debería Aparecer

### Al Ejecutar el Servidor
```
DATA_STORE_INIT: Archivo creado: data_store.log
Servidor CoAP Thread-per-Request escuchando en puerto 5683...
Ruta: coap://<IP_PC>:5683/sensors/temp (espera POST)
Modo: Thread-per-Request (sin cola FIFO)
```

### Al Recibir Datos del ESP32
```
[Thread 12345] Mensaje recibido desde 127.0.0.1:35106 (60 bytes) [Activos: 1]
[Thread 12345] URI recibido: '/sensors/temp' - Método: POST
DATA_STORE: Guardado exitoso - /sensors/temp -> {"id":"esp32-1","seq":1,"temp_c":25.5}
[Thread 12345] Respuesta enviada: 2.01 Created (50 bytes)
[Thread 12345] Procesamiento completado [Total procesados: 1]
```

### Al Usar el Cliente CLI
```
=== CoAP CLI Client Interactivo ===
coap> post 127.0.0.1 5683 /sensors/temp con '{"value":25.5}'

 Enviando request CoAP:
   Host: 127.0.0.1
   Port: 5683
   Method: post
   Path: /sensors/temp
   Mode: con
   Payload: {"value":25.5}

-> post /sensors/temp (MID=1, 45 bytes)
<- Respuesta recibida:
   Código: 65 (2.01 Created)
   Tipo: 2 (ACK - Acknowledgment)
   Message ID: 1
   Token: A1 B2
   Opciones: 0
   Payload: JSON válido recibido y guardado (12 bytes)

 Request enviado exitosamente!
```

## Archivos de Log

### server.log
```
Sun Oct  5 23:24:54 2025: Servidor CoAP Thread-per-Request escuchando en puerto 5683
Sun Oct  5 23:24:54 2025: Mensaje recibido desde 127.0.0.1:35106 (60 bytes)
Sun Oct  5 23:24:54 2025: Mensaje CoAP parseado - Ver:1 Tipo:0 Código:2
Sun Oct  5 23:24:54 2025: Respuesta CoAP enviada (50 bytes) - Código: 65 (2.01 Created)
```

### data_store.log
```
/sensors/temp	{"id":"esp32-1","seq":1,"temp_c":23.4}
/sensors/temp	{"id":"esp32-2","seq":2,"temp_c":21.3}
/sensors/temp	{"id":"esp32-3","seq":3,"temp_c":20.0}
```

## Tecnologías Utilizadas

- **C**
- **UDP sockets**
- **pthread** (threads POSIX)
- **Protocolo CoAP** (RFC 7252)
- **JSON** para datos de sensores
- **Arquitectura modular** en C

## Estructura del Proyecto

```
Proyecto CoAP/
├── src/                    # Servidor CoAP principal
│   ├── app/               # Lógica de aplicación
│   ├── networking/        # Red y protocolo CoAP
│   ├── utils/headers/     # Headers compartidos
│   ├── bin/               # Ejecutables compilados
│   └── main.c             # Punto de entrada
├── CLI/                   # Cliente CLI interactivo
├── esp_client/            # Simulador ESP32
└── README.md              # Este archivo
```

## Troubleshooting

### Error: "No such file or directory"
**Solución**: Usar WSL o Linux nativo en Windows

### Error: "Address already in use"
**Solución**: 
```bash
sudo netstat -tulpn | grep 5683
sudo kill -9 <PID>
```

### Archivos de log no se crean
**Solución**: Verificar permisos de escritura en el directorio

## Inicio Rápido

```bash
# 1. Compilar todo
cd src && make
cd ../CLI && make  
cd ../esp_client && make

# 2. Ejecutar servidor
cd ..
./src/bin/servidor1_app 5683 server.log

# 3. Probar con ESP32 (en otra terminal)
./esp_client/esp_multi 127.0.0.1 5683 /sensors/temp 1 1000 5

# 4. Ver datos guardados
tail -f data_store.log
```

¡El sistema está listo para usar!
