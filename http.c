#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback function for curl to write response data
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    HttpContext *ctx = (HttpContext *)userp;
    size_t total_size = size * nmemb;

    if (ctx->response_size + total_size >= sizeof(ctx->response_buffer)) {
        fprintf(stderr, "Error: Response buffer overflow\n");
        return 0; // This will cause curl to fail
    }

    memcpy(ctx->response_buffer + ctx->response_size, contents, total_size);
    ctx->response_size += total_size;
    ctx->response_buffer[ctx->response_size] = '\0'; // Null terminate

    return total_size;
}

bool init_curl(HttpContext *ctx) {
    // Initialize curl
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Error: Failed to initialize libcurl\n");
        return false;
    }

    // Initialize context
    memset(ctx, 0, sizeof(HttpContext));

    // Create curl handle
    ctx->curl_handle = curl_easy_init();
    if (!ctx->curl_handle) {
        fprintf(stderr, "Error: Failed to create curl handle\n");
        curl_global_cleanup();
        return false;
    }

    // Set timeout (30 seconds default)
    curl_easy_setopt(ctx->curl_handle, CURLOPT_TIMEOUT, 30);

    // Set up headers (only Content-Type for JSON)
    ctx->headers = NULL;
    ctx->headers = curl_slist_append(ctx->headers, "Content-Type: application/json");
    curl_easy_setopt(ctx->curl_handle, CURLOPT_HTTPHEADER, ctx->headers);

    // Set up response handling
    curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEDATA, ctx);

    // Enable SSL verification
    curl_easy_setopt(ctx->curl_handle, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(ctx->curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);

    // Follow redirects
    curl_easy_setopt(ctx->curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    return true;
}

bool send_request(HttpContext *ctx, const char *base_url, const char *api_key, const char *json_payload) {
    // Reset response buffer
    ctx->response_size = 0;
    ctx->response_buffer[0] = '\0';

    // Construct full URL with API key as query parameter
    char full_url[1024];
    if (snprintf(full_url, sizeof(full_url), "%s?key=%s", base_url, api_key) >= (int)sizeof(full_url)) {
        fprintf(stderr, "Error: URL too long\n");
        return false;
    }

    // Set URL
    curl_easy_setopt(ctx->curl_handle, CURLOPT_URL, full_url);

    // Set POST data
    curl_easy_setopt(ctx->curl_handle, CURLOPT_POSTFIELDS, json_payload);

    // Perform request
    CURLcode res = curl_easy_perform(ctx->curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl request failed: %s\n", curl_easy_strerror(res));
        return false;
    }

    // Check HTTP response code
    long http_code = 0;
    curl_easy_getinfo(ctx->curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code < 200 || http_code >= 300) {
        fprintf(stderr, "Error: HTTP request failed with code %ld\n", http_code);
        return false;
    }

    return true;
}

void cleanup_curl(HttpContext *ctx) {
    if (ctx->headers) {
        curl_slist_free_all(ctx->headers);
        ctx->headers = NULL;
    }

    if (ctx->curl_handle) {
        curl_easy_cleanup(ctx->curl_handle);
        ctx->curl_handle = NULL;
    }

    curl_global_cleanup();
}
