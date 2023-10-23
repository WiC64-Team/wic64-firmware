#ifndef WIC64_PROTOCOL_STANDARD_H
#define WIC64_PROTOCOL_STANDARD_H

#include "wic64.h"
#include "protocol.h"

namespace WiC64 {
    class Standard : public Protocol {
        public:
            using Protocol::Protocol;
            static const char *TAG;

            Request *createRequest(uint8_t* header);
            void setResponseHeader(uint8_t* header, uint8_t status, uint32_t size);
    };
}

#endif // WIC64_PROTOCOL_STANDARD_H