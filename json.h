#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stddef.h>

// Request Structure
typedef struct {
    char problem_text[4096];
    char language[16];
} MathRequest;

// Function declarations
bool construct_gemini_request(const MathRequest *request, char *json_buffer, size_t buffer_size);
char* escape_json_string(const char *input, char *output, size_t output_size);

#endif // JSON_H
