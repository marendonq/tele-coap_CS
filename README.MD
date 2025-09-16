# Servidor UDP Simple

## Descripción
Este es un servidor UDP básico que escucha en un puerto específico y registra todos los mensajes recibidos. El código está diseñado para recibir y analizar paquetes UDP, mostrando tanto la información del cliente como el contenido de los datos recibidos.

## Cómo funciona

### Componentes principales:
- **main.cpp**: Punto de entrada del programa que maneja argumentos de línea de comandos
- **server.cpp**: Implementación del servidor UDP que escucha y procesa mensajes
- **logger.cpp/h**: Sistema de logging que registra eventos con timestamps
- **server.h**: Declaraciones de funciones del servidor

### Flujo de funcionamiento:
1. El servidor crea un socket UDP y se enlaza al puerto especificado (por defecto 5683)
2. Entra en un bucle infinito esperando mensajes UDP
3. Cuando recibe un mensaje, muestra:
   - IP y puerto del cliente
   - Contenido en formato hexadecimal
   - Contenido como texto (si es imprimible)
4. Registra toda la actividad en un archivo de log

## Cómo compilar y probar

### Compilación:
```bash
g++ -o server main.cpp server.cpp logger.cpp
```

### Ejecución:
```bash
# Usar puerto por defecto (5683) y archivo de log por defecto (server.log)
./server

# Especificar puerto personalizado
./server 8080

# Especificar puerto y archivo de log personalizado
./server 8080 mi_log.log
```

### Pruebas:
1. **Con netcat (nc)**:
   ```bash
   echo "Hola servidor" | nc -u localhost 5683
   ```

2. **Con telnet**:
   ```bash
   telnet localhost 5683
   ```

3. **Enviar datos desde otro terminal**:
   ```bash
   echo "Mensaje de prueba" | nc -u 127.0.0.1 5683
   ```

### Verificación:
- El servidor mostrará en consola los mensajes recibidos
- Revisa el archivo `server.log` para ver el registro completo de actividad
- Los datos se muestran tanto en hexadecimal como en texto plano