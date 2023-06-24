#ifndef WIC64_UTILITIES_H
#define WIC64_UTILITIES_H

#include <cstdint>

#define WIC64_MAX_HEXDUMP_SIZE 512
#define ESP_LOG_HEXE(tag, title, data, size) log_hex(tag, ESP_LOG_ERROR, title, data, size)
#define ESP_LOG_HEXW(tag, title, data, size) log_hex(tag, ESP_LOG_WARN, title, data, size)
#define ESP_LOG_HEXI(tag, title, data, size) log_hex(tag, ESP_LOG_INFO, title, data, size)
#define ESP_LOG_HEXD(tag, title, data, size) log_hex(tag, ESP_LOG_DEBUG, title, data, size)
#define ESP_LOG_HEXV(tag, title, data, size) log_hex(tag, ESP_LOG_VERBOSE, title, data, size)

namespace WiC64 {
    void log_hex(const char* tag, esp_log_level_t level, const char* title, uint8_t *data, uint32_t size);
    void log_free_mem(void);
}
#endif // WIC64_UTILITIES_H
