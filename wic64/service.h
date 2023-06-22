#ifndef WIC64_SERVICE_H
#define WIC64_SERVICE_H

#include <cstdint>
#include "data.h"
#include "request.h"

#define LOWBYTE(UINT16) (uint8_t) ((UINT16 >> 0UL) & 0xff)
#define HIGHBYTE(UINT16) (uint8_t) ((UINT16 >> 8UL) & 0xff)

namespace WiC64 {

class Command;
class Service {
    private:
        static const uint8_t API_V1_ID = 'W';
        static const uint8_t API_V1_REQUEST_SIZE = 4;
        static const uint8_t REQUEST_HEADER_SIZE = 3;

        uint8_t* responseData;
        uint16_t responseSize;

        Request *request;
        Command *command;
        Data *response;

        void finalizeRequest(const char* result);
        static void parseRequestHeaderV1(uint8_t *header, uint16_t size);

    public:
        Service() { };
        bool supports(uint8_t apiId);

        void receiveRequest(uint8_t apiId);
        static void onRequestAborted(uint8_t* data, uint16_t bytes_received);
        void onRequestReceived(void);
        static void onRequestReceived(uint8_t* data, uint16_t size);

        void sendResponse();
        static void onResponseSizeSent(uint8_t *data, uint16_t size);
        static void onResponseAborted(uint8_t* data, uint16_t bytes_send);
        static void onResponseSent(uint8_t *data, uint16_t size);
};

}

#endif // WIC64_SERVICE_H
