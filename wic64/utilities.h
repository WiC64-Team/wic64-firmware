#ifndef WIC64_UTILITIES_H
#define WIC64_UTILITIES_H

#include <cstdint>
#include <driver/gpio.h>

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define SET_HIGH(PIN) (GPIO.out_w1ts = (1UL<<PIN))
#define SET_LOW(PIN)  (GPIO.out_w1tc = (1UL<<PIN))

#define IS_HIGH(PIN) (((GPIO.in >> PIN) & 1) == 1)
#define IS_LOW(PIN)  (((GPIO.in >> PIN) & 1) == 0)

#define LOWBYTE(UINT32) (uint8_t)((UINT32 >> 0UL) & 0xff)
#define HIGHBYTE(UINT32) (uint8_t)((UINT32 >> 8UL) & 0xff)
#define HIGHLOWBYTE(UINT32) (uint8_t)((UINT32 >> 16UL) & 0xff)
#define HIGHHIGHBYTE(UINT32) (uint8_t)((UINT32 >> 24UL) & 0xff)

#define CRLF "\x0d\x0a"

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
#define WIC64_YELLOW(str) WIC64_ANSI(33) str
#define WIC64_FORMAT_CMD WIC64_YELLOW("0x%02x")
#define WIC64_FORMAT_PROTOCOL WIC64_CYAN("0x%02x")
#define WIC64_SEPARATOR "-------------------------------------------------------------------------------"

namespace WiC64 {
    char ascii2petscii(char c);
    void ascii2petscii(char *ascii);
    void lowercase(char* lowercase);
    void replace(char* string, char c, char r);
    void log_hex(const char* tag, esp_log_level_t level, const char* title, uint8_t *data, uint32_t size);
    void log_free_mem(const char* tag, esp_log_level_t level);
    void log_task_list(const char* tag, esp_log_level_t level);
    const char* log_level_to_string(esp_log_level_t level);
}
#endif // WIC64_UTILITIES_H
