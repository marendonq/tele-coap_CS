#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct AppConfig
{
    int port;
    const char *log_file;
    const char *store_file;
    const char *resource_path;
} AppConfig;

void set_default_config(AppConfig *cfg);
void parse_config(AppConfig *cfg, int argc, char **argv);

#endif



