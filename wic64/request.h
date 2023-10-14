#ifndef WIC64_REQUEST_H
#define WIC64_REQUEST_H

#include <cstdint>
#include "data.h"

namespace WiC64 {
    class Request {
        public: static const char* TAG;

        private:
            uint8_t m_api = 0x00;
            uint8_t m_id = 0x00;
            bool m_has_payload = false;
            Data* m_payload = nullptr;

        public:
            Request(uint8_t api, uint8_t id, bool has_payload)
                : m_api(api), m_id(id), m_has_payload(has_payload) { }

            uint8_t api() { return m_api; }
            uint8_t id(void) { return m_id; };
            bool hasPayload(void) { return m_has_payload; };

            Data* payload(void) { return m_payload; }
            Data* payload(Data* payload) { m_payload = payload; return m_payload; }
    };
}

#endif // WIC64_REQUEST_H