#ifndef WIC64_UTILITIES_H
#define WIC64_UTILITIES_H

#include <cstdint>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define WIC64_MAX_HEXDUMP_SIZE 256
#define ESP_LOG_HEXE(tag, title, data, size) log_hex(tag, ESP_LOG_ERROR, title, data, size)
#define ESP_LOG_HEXW(tag, title, data, size) log_hex(tag, ESP_LOG_WARN, title, data, size)
#define ESP_LOG_HEXI(tag, title, data, size) log_hex(tag, ESP_LOG_INFO, title, data, size)
#define ESP_LOG_HEXD(tag, title, data, size) log_hex(tag, ESP_LOG_DEBUG, title, data, size)
#define ESP_LOG_HEXV(tag, title, data, size) log_hex(tag, ESP_LOG_VERBOSE, title, data, size)

#define WIC64_ANSI(c) "\033[0;" #c "m"
#define WIC64_CYAN(str) WIC64_ANSI(36) str
#define WIC64_GREEN(str) WIC64_ANSI(32) str
#define WIC64_RED(str) WIC64_ANSI(31) str
#define WIC64_WHITE(str) WIC64_ANSI(37) str
#define WIC64_FORMAT_CMD WIC64_WHITE("0x%02x")
#define WIC64_FORMAT_API WIC64_CYAN("0x%02x")

namespace WiC64 {
    void log_hex(const char* tag, esp_log_level_t level, const char* title, uint8_t *data, uint32_t size);
    void log_free_mem(const char* tag, esp_log_level_t level);
    void log_task_list(const char* tag, esp_log_level_t level);
    const char* log_level_to_string(esp_log_level_t level);
}
#endif // WIC64_UTILITIES_H
