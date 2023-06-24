#ifndef WIC64_H
#define WIC64_H

#include <cstdint>
#include <Preferences.h>
#include "esp_log.h"

namespace WiC64 {
    class WiC64 {
        public: static const char* TAG;

        private:
            Preferences preferences;

        public:
            static const uint8_t API_V1 = 'W';
            static const uint8_t API_V2 = 'I';

            static void log_level(esp_log_level_t level);

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