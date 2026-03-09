#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdbool.h>
#include <stddef.h>

// Response Structure
typedef struct {
    char *solution;  // Dynamic allocation for variable size
    size_t solution_size;
    int success;
    char error_msg[256];
} MathResponse;

// Function declarations
bool parse_gemini_response(const char *json_response, MathResponse *response);
void free_response(MathResponse *response);
void display_solution(const MathResponse *response);

#endif // RESPONSE_H
