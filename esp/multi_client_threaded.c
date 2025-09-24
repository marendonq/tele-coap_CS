// multi_client_threaded.c — Cliente CoAP multi-threaded que simula N dispositivos ESP32 reales
// Cada dispositivo corre en su propio thread
// Compilar (Linux):
//   gcc -o multi_client multi_client_threaded.c ../server/message.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>

#include "message.h"

// Estructura para datos del dispositivo
typedef struct {
    int device_id;
    const char *host;
    int port;
    const char *path;
    int interval_ms;
    long rounds;
    int confirmable_msgs;
    int *seq_ptr;
    pthread_mutex_t *seq_mutex;
    sem_t *start_semaphore;
} device_data_t;

static void add_uri_path_options(CoapMessage *msg, const char *path) {
    const char *p = path;
    while (*p == '/') p++;
    while (*p) {
        const char *start = p;
        while (*p && *p != '/') p++;
        size_t len = (size_t)(p - start);
        if (len) coap_add_option(msg, 11, (const unsigned char*)start, (unsigned short)len);
        while (*p == '/') p++;
    }
}

static double rand_temp_tenth(void) {
    int r = 180 + (rand() % (289 - 180 + 1)); // [180..289]
    return r / 10.0;
}

static int send_one_udp(int sockfd,
                        const struct sockaddr_in *server_addr,
                        const char *path,
                        const char *device_id,
                        unsigned seq,
                        double temp_c,
                        int confirmable) {
    unsigned char buffer[1024];
    CoapMessage msg;
    coap_message_init(&msg);

    msg.type = confirmable ? 0 : 1; // 0=CON, 1=NON
    msg.code = 2; // POST
    msg.message_id = (unsigned short)(rand() & 0xFFFF);

    // Token aleatorio de 2 bytes
    msg.tkl = 2;
    msg.token[0] = (unsigned char)(rand() & 0xFF);
    msg.token[1] = (unsigned char)(rand() & 0xFF);

    // Opciones de ruta
    add_uri_path_options(&msg, path);

    // Content-Format: application/json (opción 12, valor 50 según RFC)
    unsigned char cf = 50; // application/json
    coap_add_option(&msg, 12, &cf, 1);

    // Payload JSON
    char payload[160];
    snprintf(payload, sizeof(payload),
             "{\"id\":\"%s\",\"seq\":%u,\"temp_c\":%.1f}", device_id, seq, temp_c);
    coap_set_payload(&msg, (const unsigned char*)payload, (int)strlen(payload));

    int len = coap_serialize(&msg, buffer, sizeof(buffer));
    if (len <= 0) {
        fprintf(stderr, "[%s] Error al serializar CoAP\n", device_id);
        return -1;
    }

    ssize_t sent = sendto(sockfd, buffer, (size_t)len, 0,
                          (const struct sockaddr*)server_addr, sizeof(*server_addr));
    if (sent < 0) {
        fprintf(stderr, "[%s] Error al enviar UDP\n", device_id);
        return -1;
    }

    // Intentar leer una respuesta breve (no bloqueante largo)
    struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 50000; // 50 ms
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    unsigned char recv_buf[1024];
    socklen_t addr_len = sizeof(*server_addr);
    ssize_t recvd = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0,
                             (struct sockaddr*)server_addr, &addr_len);
    if (recvd > 0) {
        CoapMessage resp;
        if (coap_parse(&resp, recv_buf, (int)recvd) == 0) {
            // Resumen mínimo de respuesta
            printf("  <- resp code=%d len=%d\n", resp.code, resp.payload_len);
            for (int i = 0; i < resp.option_count; i++) {
                if (resp.options[i].value) free(resp.options[i].value);
            }
        }
    }
    return 0;
}

// Función que ejecuta cada thread de dispositivo
void* device_thread(void* arg) {
    device_data_t *data = (device_data_t*)arg;
    
    // Esperar señal de inicio
    sem_wait(data->start_semaphore);
    
    // Crear socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        fprintf(stderr, "[ESP32-%d] No se pudo crear socket UDP\n", data->device_id);
        return NULL;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    if (inet_pton(AF_INET, data->host, &server_addr.sin_addr) != 1) {
        fprintf(stderr, "[ESP32-%d] IP invalida: %s\n", data->device_id, data->host);
        close(sockfd);
        return NULL;
    }

    char dev_id[32];
    snprintf(dev_id, sizeof(dev_id), "esp32-%d", data->device_id);

    printf("[ESP32-%d] Dispositivo iniciado -> coap://%s:%d%s\n", 
           data->device_id, data->host, data->port, data->path);

    long round = 0;
    while (data->rounds == 0 || round < data->rounds) {
        double t = rand_temp_tenth();
        
        // Incrementar secuencia de forma thread-safe
        pthread_mutex_lock(data->seq_mutex);
        unsigned seq = ++(*(data->seq_ptr));
        pthread_mutex_unlock(data->seq_mutex);

        if (send_one_udp(sockfd, &server_addr, data->path, dev_id, seq, t, data->confirmable_msgs) == 0) {
            printf("[ESP32-%d] [round %ld] seq=%u -> %.1f°C\n", data->device_id, round+1, seq, t);
        }

        // Jitter aleatorio por dispositivo
        int jitter = 1000 * (5 + (rand() % 20)); // 5-24 ms
        usleep(jitter);

        if (data->interval_ms > 0) {
            usleep((useconds_t)data->interval_ms * 1000);
        }
        round++;
    }

    printf("[ESP32-%d] Dispositivo terminado\n", data->device_id);
    close(sockfd);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr,
          "Uso: %s <host> <puerto> <ruta> <num_devices> [interval_ms=5000] [rounds=0] [mode=non|con]\n", argv[0]);
        return 1;
    }

    const char *host      = argv[1];
    int port              = atoi(argv[2]);
    const char *path      = argv[3];
    int num_devices       = atoi(argv[4]);
    int interval_ms       = (argc >= 6) ? atoi(argv[5]) : 5000;
    long rounds           = (argc >= 7) ? atol(argv[6]) : 0;      // 0 = infinito
    int confirmable_msgs  = (argc >= 8 && strcmp(argv[7], "con") == 0) ? 1 : 0; // default NON

    if (num_devices <= 0) { fprintf(stderr, "num_devices debe ser > 0\n"); return 1; }

    srand((unsigned)(time(NULL) ^ getpid()));

    printf("=== Cliente CoAP Multi-threaded ===\n");
    printf("Simulando %d dispositivos %s -> coap://%s:%d%s\n",
           num_devices, confirmable_msgs ? "(CON)" : "(NON)", host, port, path);
    printf("Intervalo: %d ms, Rondas: %s\n", interval_ms, rounds ? "finitas" : "infinitas");
    printf("Cada dispositivo corre en su propio thread\n\n");

    // Crear semáforo para sincronizar inicio
    sem_t start_semaphore;
    sem_init(&start_semaphore, 0, 0);

    // Mutex para secuencia thread-safe
    pthread_mutex_t seq_mutex = PTHREAD_MUTEX_INITIALIZER;
    int shared_seq = 0;

    // Crear threads para cada dispositivo
    pthread_t *threads = malloc(num_devices * sizeof(pthread_t));
    device_data_t *device_data = malloc(num_devices * sizeof(device_data_t));

    for (int i = 0; i < num_devices; i++) {
        device_data[i].device_id = i + 1;
        device_data[i].host = host;
        device_data[i].port = port;
        device_data[i].path = path;
        device_data[i].interval_ms = interval_ms;
        device_data[i].rounds = rounds;
        device_data[i].confirmable_msgs = confirmable_msgs;
        device_data[i].seq_ptr = &shared_seq;
        device_data[i].seq_mutex = &seq_mutex;
        device_data[i].start_semaphore = &start_semaphore;

        if (pthread_create(&threads[i], NULL, device_thread, &device_data[i]) != 0) {
            fprintf(stderr, "Error creando thread para dispositivo %d\n", i + 1);
            return 1;
        }
    }

    printf("✓ %d threads de dispositivos creados\n", num_devices);
    printf("Iniciando dispositivos en 2 segundos...\n");
    sleep(2);

    // Liberar semáforo para que todos los dispositivos empiecen
    for (int i = 0; i < num_devices; i++) {
        sem_post(&start_semaphore);
    }
    printf("✓ Todos los dispositivos iniciados\n\n");

    // Esperar a que terminen todos los threads
    for (int i = 0; i < num_devices; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n✓ Todos los dispositivos terminaron\n");

    // Limpiar recursos
    free(threads);
    free(device_data);
    sem_destroy(&start_semaphore);
    pthread_mutex_destroy(&seq_mutex);

    return 0;
}
