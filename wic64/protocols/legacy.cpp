#include "legacy.h"
#include "utilities.h"

namespace WiC64 {
    const char* Legacy::TAG = "LEGACY";

    Request* Legacy::createRequest(uint8_t *header) {
        // In the legacy protocol, the payload size is not the actual payload
        // size, but rather the size of the entire request, including the four
        // header bytes. Thus we need to subtract four from the received size to
        // determine the actual payload size.

        uint32_t payload_size = (*((uint16_t*) header)) - 4;
        uint8_t command_id = header[2];

        ESP_LOGI(TAG, "Received %s request header "
                      WIC64_CYAN("[0x%02x 0x%02x ") WIC64_FORMAT_CMD WIC64_CYAN("] ")
                      WIC64_GREEN("(payload %d bytes)"),
            this->name(),
            header[0],
            header[1],
            command_id,
            payload_size);

        return new Request(this, command_id, payload_size);
    }

    void Legacy::setResponseHeader(uint8_t *header, uint8_t status, uint32_t size) {
        header[0] = HIGHBYTE(size);
        header[1] = LOWBYTE(size);

        ESP_LOGI(TAG, "Sending %s response size %d [0x%02x, 0x%02x] (big-endian)",
            this->name(),
            size,
            header[0],
            header[1]);
    }
}