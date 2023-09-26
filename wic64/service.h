#ifndef WIC64_SERVICE_H
#define WIC64_SERVICE_H

#include <cstdint>
#include "esp_event.h"
#include "data.h"
#include "request.h"
#include "callback.h"

#define LOWBYTE(UINT16) (uint8_t)((UINT16 >> 0UL) & 0xff)
#define HIGHBYTE(UINT16) (uint8_t)((UINT16 >> 8UL) & 0xff)

#ifdef ___cplusplus
extern "C"
{
#endif

    ESP_EVENT_DECLARE_BASE(SERVICE_EVENTS);

    enum
    {
        SERVICE_RESPONSE_READY,
    };

#ifdef ___cplusplus
}
#endif

namespace WiC64
{

    class Command;
    class Service
    {
    public:
        static const char *TAG;

    private:
        static const uint8_t REQUEST_HEADER_SIZE = 3;

        // In API Version1 the request size is not the argument size,
        // but rather the size of the entire payload, so we need to
        // subtract 4 to get the actual size of the argument
        // => api byte + lowbyte size + highbyte size + command id = 4
        static const uint8_t API_LAYER_1_ARGUMENT_SIZE_CORRECTION = 4;

        Request *request = NULL;
        Command *command = NULL;
        Data *response = NULL;

        int32_t bytes_remaining = 0;
        uint16_t items_remaining = 0;

    public:
        esp_event_loop_handle_t event_loop_handle;

        Service();
        esp_event_loop_handle_t eventLoop() { return event_loop_handle; }

        bool supports(uint8_t apiId);
        callback_t parseRequestHeaderCallbackFor(uint8_t api);
        void acceptRequest(uint8_t apiId);

        static void parseRequestHeader(uint8_t *header, uint16_t size);
        static void parseLegacyRequestHeader(uint8_t *header, uint16_t size);

        static void onRequestAborted(uint8_t *data, uint16_t bytes_received);
        static void onRequestReceived(uint8_t *data, uint16_t size);
        void onRequestReceived(void) { onRequestReceived(NULL, 0); }

        static void onResponseReady(void *arg, esp_event_base_t base, int32_t id, void *data);
        void sendResponse(void);

        void sendResponseHeader(void);
        static void onResponseHeaderAborted(uint8_t *data, uint16_t size);
        static void onResponseHeaderSent(uint8_t *data, uint16_t size);

        void sendQueuedResponse(void);
        static void sendQueuedResponseData(uint8_t *data, uint16_t size);
        static void sendQueuedResponseData() { sendQueuedResponseData(NULL, 0); }

        void sendStaticResponse(void);

        static void onResponseAborted(uint8_t *data, uint16_t bytes_send);
        static void onResponseSent(uint8_t *data, uint16_t size);

        void finalizeRequest(const char *message, bool success);
    };

}

#endif // WIC64_SERVICE_H
