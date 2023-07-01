#ifndef WIC64_H
#define WIC64_H

#include <cstdint>
#include "esp_log.h"

#define WIC64_QUEUE_ITEM_SIZE 512
#define WIC64_QUEUE_SIZE (0x4000 / WIC64_QUEUE_ITEM_SIZE)

#define WIC64_QUEUE_ITEMS_REQUIRED(size) \
    (size / WIC64_QUEUE_ITEM_SIZE) + \
    ((size % WIC64_QUEUE_ITEM_SIZE) ? 1 : 0)

namespace WiC64 {

    /* globalTransferBuffer - a statically allocated
     * global transfer buffer of 65536+1 bytes. This
     * is the maximum transfer length defined by the
     * protocol, plus another byte reserved in order
     * to interpret the dataas a null-terminated
     * c-string. (see Data::c_str()).
     *
     * This buffer is used for incoming requests
     * as well as outgoing responses, unless the
     * response is queued. In the latter case the
     * data is transferred via an RTOS queue of
     * 16kb.
     */
    static uint8_t transferBuffer[0x10000+1];

    class WiC64 {
        public:
            static const char* TAG;

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