#ifndef WIC64_H
#define WIC64_H

#include <cstdint>
#include "esp_log.h"

#define WIC64_QUEUE_ITEM_SIZE 128
#define WIC64_QUEUE_SIZE (0x10000 / WIC64_QUEUE_ITEM_SIZE)

#define WIC64_QUEUE_ITEMS_REQUIRED(size) \
    (size / WIC64_QUEUE_ITEM_SIZE) + \
    ((size % WIC64_QUEUE_ITEM_SIZE) ? 1 : 0)

namespace WiC64 {
    class WiC64 {
        public: static const char* TAG;

        public:
            static const uint8_t API_V1 = 'W';
            static const uint8_t API_V2 = 'I';

            static void loglevel(esp_log_level_t level);

            WiC64();
    };
}

/* The arduino header "WiFiUdp.h" needs to be included before
 * the ESP-IDF header "ipv4_addr.h" gets included. This avoids
 * a declaration conflict for IPADDR_NONE which is defined by
 * both arduino and esp-idf.
 *
 * https://github.com/espressif/arduino-esp32/issues/4405
 */
#include <WiFiUdp.h>

#endif // WIC64_H