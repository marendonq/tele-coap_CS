# Makefile para cliente CLI interactivo
CC = gcc
CFLAGS = -O2 -Wall -pthread -I../src/utils/headers

CLI_TARGET = clienteCLI
CLI_SRC = clienteCLI.c

# Fuentes del protocolo necesarias para el cliente
PROTOCOL_SRC = ../src/networking/protocol/coap_api.c \
               ../src/networking/protocol/coap_parser.c \
               ../src/networking/protocol/message.c \
               ../src/networking/protocol/logger.c \
               ../src/networking/protocol/coap_router.c \
               ../src/networking/server.c

.PHONY: all clean help

all: $(CLI_TARGET)

$(CLI_TARGET): $(CLI_SRC)
	$(CC) $(CFLAGS) -o $@ $(CLI_SRC) $(PROTOCOL_SRC)

clean:
	rm -f $(CLI_TARGET) *.o

help:
	@echo "CoAP CLI Client Interactivo - Makefile"
	@echo ""
	@echo "Comandos disponibles:"
	@echo "  make          - Compilar el cliente CLI interactivo"
	@echo "  make clean    - Limpiar archivos compilados"
	@echo "  make help     - Mostrar esta ayuda"
	@echo "  make run      - Compilar y ejecutar el cliente"
	@echo ""
	@echo "Uso del cliente:"
	@echo "  ./$(CLI_TARGET)     - Ejecutar cliente interactivo"
	@echo ""
	@echo "Comandos dentro del cliente:"
	@echo "  get <host> <port> <path> <con|non>"
	@echo "  post <host> <port> <path> <con|non> <json>"
	@echo "  put <host> <port> <path> <con|non> <json>"
	@echo "  delete <host> <port> <path> <con|non>"
	@echo "  help - Mostrar ayuda"
	@echo "  quit/exit - Salir"

run: $(CLI_TARGET)
	./$(CLI_TARGET)
