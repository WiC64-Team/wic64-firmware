#ifndef WIC64_PROTOCOL_EXTENDED_H
#define WIC64_PROTOCOL_EXTENDED_H

#include "wic64.h"
#include "protocol.h"

namespace WiC64 {
    class Extended : public Protocol {
        public:
            using Protocol::Protocol;
            static const char *TAG;

            Request *createRequest(uint8_t *header);
            void setResponseHeader(uint8_t *header, uint8_t status, uint32_t size);
    };
}

#endif // WIC64_PROTOCOL_EXTENDED_H