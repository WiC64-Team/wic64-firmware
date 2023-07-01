#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cctype>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "wic64.h"
#include "utilities.h"

namespace WiC64 {

    void log_hex(const char* tag, esp_log_level_t level, const char* title, uint8_t *data, uint32_t size) {
        static char hint[64];
        int max = size;

        if (size > WIC64_MAX_HEXDUMP_SIZE) {
            max = WIC64_MAX_HEXDUMP_SIZE;
            snprintf(hint, 64, " (only first %d bytes shown)", WIC64_MAX_HEXDUMP_SIZE);
        }
        else {
            hint[0] = '\0';
        }

        ESP_LOG_LEVEL(level, tag, "HEXDUMP %s:%s", title, ((size > max) ? hint : ""));
        ESP_LOG_BUFFER_HEXDUMP(tag, data, max, level);
    }

    void log_free_mem(const char* tag, esp_log_level_t level) {
        uint32_t free_heap_size = esp_get_free_heap_size();
        uint32_t minimum_free_heap_size = esp_get_minimum_free_heap_size();

        ESP_LOG_LEVEL(level, tag, "Free heap (current) : %d bytes = %dkb",
            free_heap_size, free_heap_size/1024);
        ESP_LOG_LEVEL(level, tag, "Free heap (minimum) : %d bytes = %dkb",
            minimum_free_heap_size, minimum_free_heap_size/1024);
    }

    void log_task_list(const char* tag, esp_log_level_t level) {
        static char stats[1024];
        vTaskList(stats);
        ESP_LOG_LEVEL(level, tag, "\n%s", stats);
    }

    const char* log_level_to_string(esp_log_level_t level) {
        if (level == ESP_LOG_NONE) return "NONE";
        if (level == ESP_LOG_ERROR) return "ERROR";
        if (level == ESP_LOG_WARN) return "WARN";
        if (level == ESP_LOG_INFO) return "INFO";
        if (level == ESP_LOG_DEBUG) return "DEBUG";
        if (level == ESP_LOG_VERBOSE) return "VERBOSE";
        return "UKNOWN";
    }
}
