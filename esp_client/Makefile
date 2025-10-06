# Makefile para simulador ESP32
CC = gcc
CFLAGS = -O2 -Wall -pthread -I../src/utils/headers

ESP_TARGET = esp_multi
ESP_SRC = multi_client_threaded.c

# Fuentes del protocolo necesarias para el simulador ESP
PROTOCOL_SRC = ../src/networking/protocol/message.c

.PHONY: all clean help

all: $(ESP_TARGET)

$(ESP_TARGET): $(ESP_SRC)
	$(CC) $(CFLAGS) -o $@ $(ESP_SRC) $(PROTOCOL_SRC)

clean:
	rm -f $(ESP_TARGET) *.o

help:
	@echo "ESP32 Simulator - Makefile"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar el simulador ESP32"
	@echo "  make clean    - Limpiar archivos compilados"
	@echo "  make help     - Mostrar esta ayuda"
	@echo "  make run      - Compilar y ejecutar el simulador"
	@echo ""
	@echo "Uso del simulador:"
	@echo "  ./$(ESP_TARGET) <host> <port> <path> <device_count> <interval_ms> <rounds>"
	@echo ""
	@echo "Ejemplo:"
	@echo "  ./$(ESP_TARGET) 127.0.0.1 5683 /sensors/temp 5 1000 10"

run: $(ESP_TARGET)
	@echo "Ejecutando simulador ESP32..."
	@echo "Uso: ./$(ESP_TARGET) <host> <port> <path> <device_count> <interval_ms> <rounds>"
	@echo "Ejemplo: ./$(ESP_TARGET) 127.0.0.1 5683 /sensors/temp 5 1000 10"
