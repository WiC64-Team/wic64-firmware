#include "extended.h"
#include "utilities.h"

namespace WiC64 {
    const char* Extended::TAG = "EXTENDED";

    Request *Extended::createRequest(uint8_t *header) {
        uint8_t command_id = header[0];
        uint32_t payload_size = (*((uint32_t*) (header+1))) ;

        ESP_LOGI(TAG, "Received %s request header "
                      WIC64_CYAN("[") WIC64_FORMAT_CMD WIC64_CYAN(" 0x%02x 0x%02x 0x%02x 0x%02x] ")
                      WIC64_GREEN("(payload %d bytes)"),
            this->name(),
            command_id,
            header[1],
            header[2],
            header[3],
            header[4],
            payload_size);

        return new Request(this, command_id, payload_size);
    }

    void Extended::setResponseHeader(uint8_t *header, uint8_t status, uint32_t size) {
        header[0] = status;
        header[1] = LOWBYTE(size);
        header[2] = HIGHBYTE(size);
        header[3] = HIGHLOWBYTE(size);
        header[4] = HIGHHIGHBYTE(size);

        ESP_LOGI(TAG, "Sending %s response header (status %d, size: %d): [0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x]",
            this->name(),
            status,
            size,
            header[0],
            header[1],
            header[2],
            header[3],
            header[4]);
    }
}