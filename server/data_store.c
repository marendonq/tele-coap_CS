#include "data_store.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Entry {
    char *key;   // uri_path
    char *value; // json
    struct Entry *next;
} Entry;

static Entry *head = NULL;
static pthread_mutex_t store_mutex = PTHREAD_MUTEX_INITIALIZER;
static char store_file[256] = {0};

static void set_in_memory(const char *key, const char *value) {
    Entry *e = head;
    while (e) {
        if (strcmp(e->key, key) == 0) {
            free(e->value);
            e->value = strdup(value);
            return;
        }
        e = e->next;
    }
    Entry *n = (Entry*)malloc(sizeof(Entry));
    n->key = strdup(key);
    n->value = strdup(value);
    n->next = head;
    head = n;
}

static const char* get_in_memory(const char *key) {
    Entry *e = head;
    while (e) {
        if (strcmp(e->key, key) == 0) return e->value;
        e = e->next;
    }
    return NULL;
}

static void rewrite_file(void) {
    if (!store_file[0]) return;
    FILE *f = fopen(store_file, "w");
    if (!f) return;
    for (Entry *e = head; e; e = e->next) {
        fprintf(f, "%s\t%s\n", e->key, e->value);
    }
    fclose(f);
}

int data_store_init(const char *filepath) {
    if (!filepath) return -1;
    strncpy(store_file, filepath, sizeof(store_file)-1);
    store_file[sizeof(store_file)-1] = '\0';

    // Cargar pares key\tjson por línea si existe
    FILE *f = fopen(store_file, "r");
    if (!f) return 0; // no existe aún
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *tab = strchr(line, '\t');
        if (!tab) continue;
        *tab = '\0';
        char *key = line;
        char *val = tab + 1;
        // quitar salto
        size_t L = strlen(val);
        if (L > 0 && (val[L-1] == '\n' || val[L-1] == '\r')) val[L-1] = '\0';
        pthread_mutex_lock(&store_mutex);
        set_in_memory(key, val);
        pthread_mutex_unlock(&store_mutex);
    }
    fclose(f);
    return 0;
}

int data_store_set(const char *uri_path, const char *json_payload) {
    if (!uri_path || !json_payload) return -1;
    pthread_mutex_lock(&store_mutex);
    set_in_memory(uri_path, json_payload);
    // Añadir/actualizar en archivo (append simple; última entrada es la válida)
    if (store_file[0]) {
        FILE *f = fopen(store_file, "a");
        if (f) {
            fprintf(f, "%s\t%s\n", uri_path, json_payload);
            fclose(f);
        }
    }
    pthread_mutex_unlock(&store_mutex);
    return 0;
}

int data_store_get(const char *uri_path, char *out_payload, size_t out_size) {
    if (!uri_path || !out_payload || out_size == 0) return -1;
    pthread_mutex_lock(&store_mutex);
    const char *val = get_in_memory(uri_path);
    int n = 0;
    if (val) {
        n = (int)snprintf(out_payload, out_size, "%s", val);
    }
    pthread_mutex_unlock(&store_mutex);
    return n; // 0 si no existe
}

int data_store_delete(const char *uri_path) {
    if (!uri_path) return -1;
    pthread_mutex_lock(&store_mutex);
    Entry *prev = NULL, *cur = head;
    int removed = 0;
    while (cur) {
        if (strcmp(cur->key, uri_path) == 0) {
            if (prev) prev->next = cur->next; else head = cur->next;
            free(cur->key);
            free(cur->value);
            free(cur);
            removed = 1;
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if (removed) rewrite_file();
    pthread_mutex_unlock(&store_mutex);
    return 0;
}

void data_store_cleanup(void) {
    pthread_mutex_lock(&store_mutex);
    Entry *e = head;
    while (e) {
        Entry *n = e->next;
        free(e->key);
        free(e->value);
        free(e);
        e = n;
    }
    head = NULL;
    pthread_mutex_unlock(&store_mutex);
}


