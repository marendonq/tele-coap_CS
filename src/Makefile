# New Makefile to build server from source layout without modifying existing folders

CC = gcc
CFLAGS = -O2 -Wall -pthread \
  -Iutils/headers

# Output directory for binaries built via this Makefile
BIN_DIR = ./bin

# Source files: everything under src now (protocol included)
SRV_SRC = \
  main.c \
  app/config.c \
  app/handlers.c \
  app/routes.c \
  app/persistence.c \
  networking/server.c \
  networking/protocol/coap_api.c \
  networking/protocol/coap_parser.c \
  networking/protocol/coap_router.c \
  networking/protocol/data_store.c \
  networking/protocol/message.c \
  networking/protocol/logger.c

all: prep_dirs servidor1_app

prep_dirs:
	@mkdir -p $(BIN_DIR)

servidor1_app: $(SRV_SRC)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $(SRV_SRC)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean prep_dirs


