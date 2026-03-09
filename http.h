#ifndef HTTP_H
#define HTTP_H

#include <curl/curl.h>
#include <stdbool.h>

// HTTP Context Structure
typedef struct {
    CURL *curl_handle;
    struct curl_slist *headers;
    char response_buffer[8192];
    size_t response_size;
} HttpContext;

// Function declarations
bool init_curl(HttpContext *ctx);
bool send_request(HttpContext *ctx, const char *base_url, const char *api_key, const char *json_payload);
void cleanup_curl(HttpContext *ctx);

#endif // HTTP_H
