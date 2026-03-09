#include "config.h"
#include "http.h"
#include "json.h"
#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "config.ini"
#define MAX_INPUT_SIZE 4096

int main(void) {
    Config config;
    HttpContext http_ctx;
    MathRequest request;
    MathResponse response;
    char json_payload[8192];

    // Phase 1: Initialization
    printf("Math Solver CLI - Loading configuration...\n");

    // Load configuration
    if (!load_config(CONFIG_FILE, &config)) {
        fprintf(stderr, "Failed to load configuration\n");
        return EXIT_FAILURE;
    }

    // Validate configuration
    if (!validate_config(&config)) {
        fprintf(stderr, "Invalid configuration\n");
        return EXIT_FAILURE;
    }

    // Initialize curl
    if (!init_curl(&http_ctx, config.timeout_seconds)) {
        fprintf(stderr, "Failed to initialize HTTP client\n");
        return EXIT_FAILURE;
    }

    // Phase 2: Input Phase
    printf("Enter your math problem (max %d characters):\n", MAX_INPUT_SIZE - 1);

    if (!fgets(request.problem_text, sizeof(request.problem_text), stdin)) {
        fprintf(stderr, "Error reading input\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    // Remove trailing newline
    size_t input_len = strcspn(request.problem_text, "\n");
    bool was_truncated = (input_len == sizeof(request.problem_text) - 1 && request.problem_text[input_len] != '\n');
    request.problem_text[input_len] = '\0';

    // Validate input
    if (strlen(request.problem_text) == 0) {
        fprintf(stderr, "Error: Empty input\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    if (was_truncated) {
        fprintf(stderr, "Error: Input too long\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    // Set language
    strcpy(request.language, config.language);

    // Phase 3: API Request Phase
    printf("Constructing request and contacting Gemini API...\n");

    // Construct JSON payload
    if (!construct_gemini_request(&request, json_payload, sizeof(json_payload))) {
        fprintf(stderr, "Failed to construct request\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    // Send request
    if (!send_request(&http_ctx, config.endpoint, config.api_key, json_payload)) {
        fprintf(stderr, "Failed to send request\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    // Phase 4: Response Processing Phase
    printf("Processing response...\n");

    // Parse response
    if (!parse_gemini_response(http_ctx.response_buffer, &response)) {
        fprintf(stderr, "Failed to parse response\n");
        cleanup_curl(&http_ctx);
        return EXIT_FAILURE;
    }

    // Phase 5: Output Phase
    display_solution(&response);

    // Cleanup
    free_response(&response);
    cleanup_curl(&http_ctx);

    return EXIT_SUCCESS;
}
