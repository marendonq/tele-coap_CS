# Compilador y flags
CC = gcc
CFLAGS = -pthread

# Ejecutables
PROGS = coap_server multi_client coap_cli

all: $(PROGS)

coap_server: server/main.c server/coap_api.c server/server_thread_per_request.c server/coap_parser.c server/coap_router.c server/data_store.c server/message.c server/logger.c
	$(CC) $(CFLAGS) -o $@ $^

multi_client: esp/multi_client_threaded.c esp/message.c
	$(CC) $(CFLAGS) -o $@ $^ 

coap_cli: client/coap_cli.c client/message.c
	$(CC) -o $@ $^

clean:
	rm -f $(PROGS) *.o

