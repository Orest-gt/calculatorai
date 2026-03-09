#include "response.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Improved JSON parser for OpenAI chat completions response
// More robust parsing of choices[0].message.content
static char* extract_text_value(const char *json, const char *key_path __attribute__((unused))) {
    // Find "choices"
    const char *choices = strstr(json, "\"choices\"");
    if (!choices) return NULL;

    // Find the start of the choices array
    const char *array_start = strchr(choices, '[');
    if (!array_start) return NULL;

    // Find the first object in the array
    const char *object_start = strchr(array_start, '{');
    if (!object_start) return NULL;

    // Find "message" within this object
    const char *message = strstr(object_start, "\"message\"");
    if (!message) return NULL;

    // Find the message object start
    const char *message_obj = strchr(message, '{');
    if (!message_obj) return NULL;

    // Find "content" within the message object
    const char *content = strstr(message_obj, "\"content\"");
    if (!content) return NULL;

    // Find the colon and then the opening quote
    const char *colon = strchr(content, ':');
    if (!colon) return NULL;

    const char *value_start = colon + 1;
    while (*value_start && *value_start != '"') value_start++;
    if (*value_start != '"') return NULL;

    value_start++; // Skip the opening quote
    const char *value_end = value_start;

    // Find the closing quote, handling escaped quotes
    while (*value_end) {
        if (*value_end == '"') {
            // Check if it's escaped
            int backslash_count = 0;
            const char *check = value_end - 1;
            while (check >= value_start && *check == '\\') {
                backslash_count++;
                check--;
            }
            if (backslash_count % 2 == 0) {
                // Not escaped, this is the end
                break;
            }
        }
        value_end++;
    }

    if (*value_end != '"') return NULL;

    // Allocate and copy the text
    size_t text_len = value_end - value_start;
    char *text_value = (char*)malloc(text_len + 1);
    if (!text_value) return NULL;

    // Handle escape sequences
    size_t i = 0, j = 0;
    for (i = 0; i < text_len; i++) {
        if (value_start[i] == '\\') {
            i++; // Skip backslash
            switch (value_start[i]) {
                case 'n': text_value[j++] = '\n'; break;
                case 'r': text_value[j++] = '\r'; break;
                case 't': text_value[j++] = '\t'; break;
                case '"': text_value[j++] = '"'; break;
                case '\\': text_value[j++] = '\\'; break;
                default: text_value[j++] = value_start[i]; break;
            }
        } else {
            text_value[j++] = value_start[i];
        }
    }
    text_value[j] = '\0';

    return text_value;
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
