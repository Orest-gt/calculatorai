#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

// Configuration Structure
typedef struct {
    char api_key[256];
    char language[16];
    char endpoint[256];
    int timeout_seconds;
} Config;

// Function declarations
bool load_config(const char *config_path, Config *config);
bool validate_config(const Config *config);

#endif // CONFIG_H
