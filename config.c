#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper function to strip surrounding quotes
static void strip_quotes(char *str) {
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        memmove(str, str + 1, len - 2);
        str[len - 2] = '\0';
    }
}

// Helper function to trim whitespace
static char* trim_whitespace(char* str) {
    if (!str) return NULL;

    // Trim leading space
    char* start = str;
    while(isspace((unsigned char)*start)) start++;

    if(*start == 0) {
        *str = '\0';
        return str;
    }

    // Trim trailing space
    char* end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;

    // Shift the trimmed string to the beginning
    size_t trimmed_len = end - start + 1;
    if (start != str) {
        memmove(str, start, trimmed_len);
    }

    // Null terminate
    str[trimmed_len] = '\0';

    return str;
}

static bool parse_ini_line(const char* line, char* key, char* value) {
    const char* equals = strchr(line, '=');
    if (!equals) return false;

    size_t key_len = equals - line;
    if (key_len >= 256) return false;

    strncpy(key, line, key_len);
    key[key_len] = '\0';
    trim_whitespace(key);

    const char* value_start = equals + 1;
    size_t value_len = strlen(value_start);
    if (value_len >= 256) return false; // Value too long

    memcpy(value, value_start, value_len);
    value[value_len] = '\0';
    trim_whitespace(value);

    return true;
}

bool load_config(const char *config_path, Config *config) {
    FILE *file = fopen(config_path, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open config file '%s'\n", config_path);
        return false;
    }

    // Set defaults
    strcpy(config->language, "en");
    config->timeout_seconds = 30;
    strcpy(config->endpoint, "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.1-flash-lite:generateContent");

    char line[512];
    char section[64] = "";
    bool in_gemini_section = false;

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;

        // Skip empty lines and comments
        if (strlen(trim_whitespace(line)) == 0 || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Section header
        if (line[0] == '[') {
            char* end = strchr(line, ']');
            if (end) {
                *end = '\0';
                char* section_start = line + 1;
                size_t section_len = end - section_start;
                if (section_len < sizeof(section)) {
                    strncpy(section, section_start, section_len);
                    section[section_len] = '\0';
                    in_gemini_section = (strcmp(section, "gemini") == 0);
                } else {
                    fprintf(stderr, "Warning: Section name too long, ignoring\n");
                }
            }
            continue;
        }

        // Parse key-value pairs only in [gemini] section
        if (in_gemini_section) {
            char key[256];
            char value[256];

            if (parse_ini_line(line, key, value)) {
                if (strcmp(key, "api_key") == 0) {
                    char temp_value[256];
                    strncpy(temp_value, value, sizeof(temp_value) - 1);
                    temp_value[sizeof(temp_value) - 1] = '\0';
                    strip_quotes(temp_value);
                    strncpy(config->api_key, temp_value, sizeof(config->api_key) - 1);
                    config->api_key[sizeof(config->api_key) - 1] = '\0';
                } else if (strcmp(key, "language") == 0) {
                    strncpy(config->language, value, sizeof(config->language) - 1);
                    config->language[sizeof(config->language) - 1] = '\0';
                } else if (strcmp(key, "timeout") == 0) {
                    config->timeout_seconds = atoi(value);
                } else if (strcmp(key, "endpoint") == 0) {
                    strncpy(config->endpoint, value, sizeof(config->endpoint) - 1);
                    config->endpoint[sizeof(config->endpoint) - 1] = '\0';
                }
            }
        }
    }

    fclose(file);
    return true;
}

bool validate_config(const Config *config) {
    if (strlen(config->api_key) == 0) {
        fprintf(stderr, "Error: API key is required\n");
        return false;
    }

    if (strlen(config->language) == 0) {
        fprintf(stderr, "Error: Language is required\n");
        return false;
    }

    if (config->timeout_seconds <= 0 || config->timeout_seconds > 300) {
        fprintf(stderr, "Error: Timeout must be between 1 and 300 seconds\n");
        return false;
    }

    if (strlen(config->endpoint) == 0) {
        fprintf(stderr, "Error: Endpoint URL is required\n");
        return false;
    }

    return true;
}
