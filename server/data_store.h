#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa el almacén con un archivo de respaldo (persistencia simple por líneas)
// Retorna 0 en éxito.
int data_store_init(const char *filepath);

// Guarda (en memoria y archivo) el payload JSON asociado a un recurso (uri_path)
// Retorna 0 en éxito.
int data_store_set(const char *uri_path, const char *json_payload);

// Obtiene el último payload JSON asociado a un recurso
// Retorna número de bytes copiados a out_payload, 0 si no existe, -1 error.
int data_store_get(const char *uri_path, char *out_payload, size_t out_size);

// Elimina la entrada asociada al recurso (memoria y archivo, reescribiendo)
// Retorna 0 en éxito, 0 si no existía, -1 error.
int data_store_delete(const char *uri_path);

// Libera recursos. No borra el archivo.
void data_store_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif


