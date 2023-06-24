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
    public: static const char* TAG;

    private:
        static const uint8_t REQUEST_HEADER_SIZE = 3;

        // In API Version1 the request size is not the argument size,
        // but rather the size of the entire payload, so we need to
        // subtract 4 to get the actual size of the argument
        // => api byte + lowbyte size + highbyte size + command id = 4
        static const uint8_t API_V1_ARGUMENT_SIZE_CORRECTION = 4;

        uint8_t* responseData;
        uint16_t responseSize;

        Request *request;
        Command *command;
        Data *response;

        void finalizeRequest(const char* message, bool success);
        static void parseRequestHeaderVersion1(uint8_t *header, uint16_t size);

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
