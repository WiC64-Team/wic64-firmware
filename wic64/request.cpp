#include <cstdint>
#include <cstdlib>
#include "esp_log.h"

#include "request.h"
#include "data.h"

namespace WiC64 {
    const char* Request::TAG = "REQUEST";

    Request::~Request() {
        delete m_payload;
    }

    Data *Request::payload(uint8_t *data, uint32_t size)  {
        m_payload->set(data, size);
        return m_payload;
    }
}