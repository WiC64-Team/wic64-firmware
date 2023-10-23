#ifndef WIC64_REQUEST_H
#define WIC64_REQUEST_H

#include <cstdint>

#include "protocol.h"
#include "data.h"

namespace WiC64 {
    class Protocol;

    class Request {
        public: static const char* TAG;

        private:
            Protocol* m_protocol = nullptr;
            uint8_t m_id = 0x00;
            Data* m_payload = new Data();

        public:
            Request(Protocol *protocol, uint8_t id, uint32_t payload_size)
                : m_protocol(protocol), m_id(id) {
                    m_payload->size(payload_size);
            }

            ~Request();

            Protocol* protocol() { return m_protocol; }
            uint8_t id(void) { return m_id; };
            bool hasPayload(void) { return m_payload->size() > 0; };

            Data* payload(void) { return m_payload; }
            Data* payload(uint8_t *data, uint32_t size);
    };
}

#endif // WIC64_REQUEST_H