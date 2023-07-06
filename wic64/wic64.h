#ifndef WIC64_H
#define WIC64_H

#include <cstdint>
#include "esp_log.h"

#define WIC64_QUEUE_ITEM_SIZE 1024
#define WIC64_QUEUE_SIZE (0x8000 / WIC64_QUEUE_ITEM_SIZE)

#define WIC64_QUEUE_ITEMS_REQUIRED(size) \
    (size / WIC64_QUEUE_ITEM_SIZE) + \
    ((size % WIC64_QUEUE_ITEM_SIZE) ? 1 : 0)

namespace WiC64 {

    /* Global transfer buffer of 65536+1 bytes. This
     * is the maximum transfer length defined by the
     * protocol, plus another byte reserved in order
     * to interpret the data as a null-terminated
     * c-string. (see Data::c_str()).
     *
     * This buffer is used for incoming requests
     * as well as outgoing responses, unless the
     * response is queued. In the latter case the
     * data is transferred via an RTOS queue of
     * 16kb.
     *
     * The reason this buffer is not statically
     * allocated is that on the esp32, the amount
     * of statically allocatable memory is limited
     * due to hard coded memory locations in the
     * esps roms, so the general advice is to
     * dynamically allocate large buffers even
     * though a static allocation would seem the more
     * appropriate thing to do.
     *
     * Since this buffer is allocated early in the
     * WiC64 constructor and never freed, this will
     * not contribute to any heap fragmentation.
     */
    extern uint8_t *transferBuffer;

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