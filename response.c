#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Simple JSON parser for Gemini response
// Expected structure: {"candidates":[{"content":{"parts":[{"text":"solution"}]}}]}
static char* extract_text_value(const char *json, const char *key_path __attribute__((unused))) {
    // This is a very basic JSON parser - in production you'd want a proper JSON library
    // But for this simple use case, we'll extract the text field manually

    const char *candidates_start = strstr(json, "\"candidates\"");
    if (!candidates_start) return NULL;

    const char *text_start = strstr(candidates_start, "\"text\"");
    if (!text_start) return NULL;

    // Find the colon after "text"
    const char *colon = strchr(text_start, ':');
    if (!colon) return NULL;

    // Skip colon and whitespace
    const char *value_start = colon + 1;
    while (*value_start && (isspace(*value_start) || *value_start == '"')) value_start++;

    if (*value_start != '"') return NULL;

    // Find the end quote
    const char *value_end = value_start + 1;
    while (*value_end && *value_end != '"') {
        if (*value_end == '\\') value_end++; // Skip escaped characters
        value_end++;
    }

    if (*value_end != '"') return NULL;

    // Allocate and copy the text
    size_t text_len = value_end - value_start - 1;
    char *text = (char*)malloc(text_len + 1);
    if (!text) return NULL;

    // Handle escape sequences
    size_t i = 0, j = 0;
    for (i = 1; i < text_len; i++) { // Start from 1 to skip opening quote
        if (value_start[i] == '\\') {
            i++; // Skip backslash
            switch (value_start[i]) {
                case 'n': text[j++] = '\n'; break;
                case 'r': text[j++] = '\r'; break;
                case 't': text[j++] = '\t'; break;
                case '"': text[j++] = '"'; break;
                case '\\': text[j++] = '\\'; break;
                default: text[j++] = value_start[i]; break;
            }
        } else {
            text[j++] = value_start[i];
        }
    }
    text[j] = '\0';

    return text;
}

bool parse_gemini_response(const char *json_response, MathResponse *response) {
    // Initialize response
    response->solution = NULL;
    response->solution_size = 0;
    response->success = 0;
    response->error_msg[0] = '\0';

    if (!json_response || strlen(json_response) == 0) {
        strcpy(response->error_msg, "Empty response");
        return false;
    }

    // Check for error indicators
    if (strstr(json_response, "\"error\"")) {
        // Try to extract error message
        const char *error_start = strstr(json_response, "\"message\"");
        if (error_start) {
            const char *colon = strchr(error_start, ':');
            if (colon) {
                const char *msg_start = colon + 1;
                while (*msg_start && (isspace(*msg_start) || *msg_start == '"')) msg_start++;

                const char *msg_end = msg_start;
                while (*msg_end && *msg_end != '"') msg_end++;

                size_t msg_len = msg_end - msg_start;
                if (msg_len > 0 && msg_len < sizeof(response->error_msg)) {
                    strncpy(response->error_msg, msg_start, msg_len);
                    response->error_msg[msg_len] = '\0';
                } else {
                    strcpy(response->error_msg, "API error occurred");
                }
            }
        } else {
            strcpy(response->error_msg, "API error occurred");
        }
        return false;
    }

    // Extract solution text
    char *solution_text = extract_text_value(json_response, "candidates[0].content.parts[0].text");
    if (!solution_text) {
        strcpy(response->error_msg, "Failed to parse response");
        return false;
    }

    response->solution = solution_text;
    response->solution_size = strlen(solution_text);
    response->success = 1;

    return true;
}

void free_response(MathResponse *response) {
    if (response->solution) {
        free(response->solution);
        response->solution = NULL;
        response->solution_size = 0;
    }
}

void display_solution(const MathResponse *response) {
    if (response->success && response->solution) {
        printf("\n%s\n", response->solution);
    } else {
        printf("Error: %s\n", response->error_msg);
    }
}
