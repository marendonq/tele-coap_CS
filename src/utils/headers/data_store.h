#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int data_store_init(const char *filepath);
int data_store_set(const char *uri_path, const char *json_payload);
int data_store_get(const char *uri_path, char *out_payload, size_t out_size);
int data_store_delete(const char *uri_path);
void data_store_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif



