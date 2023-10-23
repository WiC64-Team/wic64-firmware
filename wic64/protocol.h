#ifndef WIC64_PROTOCOL_H
#define WIC64_PROTOCOL_H

#include <map>

#include "wic64.h"
#include "request.h"

namespace WiC64 {
    class Request;

    class Protocol {
        private:
            static const std::map<uint8_t, Protocol*> m_protocols;

            uint8_t m_id;
            const char* m_name;
            uint8_t m_requestHeaderSize;
            uint8_t m_responseHeaderSize;

        public:
            static const uint8_t LEGACY   = 'W';
            static const uint8_t STANDARD = 'R';
            static const uint8_t EXTENDED = 'E';

            static const uint8_t MAX_REQUEST_HEADER_SIZE = 5;
            static const uint8_t MAX_RESPONSE_HEADER_SIZE = 5;

            Protocol()=default;

            Protocol(const uint8_t id,
                    const char* name,
                    const uint8_t requestHeaderSize,
                    const uint8_t responseHeaderSize) :
                    m_id(id),
                    m_name(name),
                    m_requestHeaderSize(requestHeaderSize),
                    m_responseHeaderSize(responseHeaderSize) { }

            static bool exists(uint8_t id);
            static Protocol* get(uint8_t id);

            uint8_t id(void) { return m_id; }
            const char* name(void) { return m_name; }
            uint8_t requestHeaderSize(void) { return m_requestHeaderSize; }
            uint8_t responseHeaderSize(void) { return m_responseHeaderSize; }

            virtual Request* createRequest(uint8_t *header);
            virtual void setResponseHeader(uint8_t *header, uint8_t status, uint32_t size);
    };
}

#endif // WIC64_PROTOCOL_H