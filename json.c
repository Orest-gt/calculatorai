#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to escape JSON strings
char* escape_json_string(const char *input, char *output, size_t output_size) {
    size_t i = 0, j = 0;

    if (!input || !output || output_size == 0) {
        return NULL;
    }

    while (input[i] != '\0' && j < output_size - 2) { // Leave room for null terminator
        switch (input[i]) {
            case '"':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = '"';
                break;
            case '\\':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = '\\';
                break;
            case '\b':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = 'b';
                break;
            case '\f':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = 'f';
                break;
            case '\n':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = 'n';
                break;
            case '\r':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = 'r';
                break;
            case '\t':
                if (j + 2 >= output_size) break;
                output[j++] = '\\';
                output[j++] = 't';
                break;
            default:
                if ((unsigned char)input[i] < 32) {
                    // Escape control characters
                    if (j + 6 >= output_size) break;
                    snprintf(output + j, 7, "\\u%04x", (unsigned char)input[i]);
                    j += 6;
                } else {
                    output[j++] = input[i];
                }
                break;
        }
        i++;
    }

    output[j] = '\0';
    return output;
}

bool construct_gemini_request(const MathRequest *request, char *json_buffer, size_t buffer_size) {
    // Escape the problem text - allocate enough for worst-case 6x expansion
    char escaped_problem[4096 * 6 + 1];
    if (!escape_json_string(request->problem_text, escaped_problem, sizeof(escaped_problem))) {
        fprintf(stderr, "Error: Failed to escape problem text\n");
        return false;
    }

    // Escape the language string
    char escaped_language[16 * 6 + 1];
    if (!escape_json_string(request->language, escaped_language, sizeof(escaped_language))) {
        fprintf(stderr, "Error: Failed to escape language\n");
        return false;
    }

    // Construct the JSON payload
    int written = snprintf(json_buffer, buffer_size,
        "{"
        "\"system_instruction\":{"
        "\"parts\":["
        "{"
        "\"text\":\"Give ready to write solution, fully explained and answered. No conversation with the user, only straight up output. Be professional and do nothing else than what proposed. Answer in %s.\""
        "}"
        "]"
        "},"
        "\"contents\":["
        "{"
        "\"parts\":["
        "{"
        "\"text\":\"%s\""
        "}"
        "]"
        "}"
        "]"
        "}",
        escaped_language,
        escaped_problem
    );

    if (written < 0 || (size_t)written >= buffer_size) {
        fprintf(stderr, "Error: JSON buffer overflow\n");
        return false;
    }

    return true;
}
