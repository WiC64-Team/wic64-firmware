#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cctype>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32-hal-log.h"
#include "utilities.h"

void hexdump(const char* title, uint8_t *data, uint32_t size) {
    if(esp_log_level_get(NULL) < ESP_LOG_VERBOSE) return;

    const uint8_t bytes_per_line = 8;
    const uint8_t chars_per_line = strlen(title) + 2 + (bytes_per_line * 4) + 10 + 1 + 2;

    uint64_t num_lines = (size / bytes_per_line) + ((size % bytes_per_line) ? 1 : 0);

    uint64_t addr;
    uint64_t data_index;
    char* cursor;

    for (uint64_t line_index = 0; line_index < num_lines; line_index++) {
        char *line = (char*) calloc(chars_per_line, sizeof(char));
        cursor = line;

        cursor += snprintf(line, strlen(title)+3, "%s: ", title);

        addr = line_index * bytes_per_line;
        cursor += snprintf(cursor, 9, "%08llx", addr);

        cursor += snprintf(cursor, 4, "| ");

        for ( uint8_t i=0; i<bytes_per_line; i++) {
            data_index = line_index * bytes_per_line + i;

            if (data_index < size) {
                cursor += snprintf(cursor, 3, "%02x", data[data_index]);
                cursor += snprintf(cursor, 2, " ");
            }
            else {
                cursor += snprintf(cursor, 4, "   ");
            }
        }
        cursor += snprintf(cursor, 2, "|");

        for (uint8_t i=0; i<bytes_per_line; i++) {
            data_index = line_index * bytes_per_line + i;
            if (data_index < size) {
                uint8_t c = data[data_index];
                cursor[i] = isprint((char)c) ? c : '.';
            }
            else {
                cursor[i] = ' ';
            }
        }
        cursor += bytes_per_line;
        cursor += snprintf(cursor, 2, "|");

        log_v("%s", line);
        vTaskDelay(1);

        free(line);
    }
}

void log_free_mem(void) {
    uint32_t free_heap_size = esp_get_free_heap_size();
    uint32_t minimum_free_heap_size = esp_get_minimum_free_heap_size();

    log_d("Free heap (current) : %d bytes = %dkb",
        free_heap_size, free_heap_size/1024);
    log_d("Free heap (minimum) : %d bytes = %dkb",
        minimum_free_heap_size, minimum_free_heap_size/1024);
}
