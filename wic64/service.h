#ifndef WIC64_SERVICE_H
#define WIC64_SERVICE_H

#include <cstdint>
#include "esp_event.h"

#include "protocol.h"
#include "data.h"
#include "request.h"
#include "callback.h"

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

namespace WiC64 {
    class Command;
    class Service {
        public:
            static const char *TAG;

        private:
            Protocol* protocol;

            // => protocol byte + lowbyte size + highbyte size + command id = 4
            static const uint8_t LEGACY_PROTOCOL_PAYLOAD_SIZE_CORRECTION = 4;

            Request *request = NULL;
            Command *command = NULL;
            Data *response = NULL;

            int32_t bytes_remaining = 0;
            uint32_t items_remaining = 0;

        public:
            esp_event_loop_handle_t event_loop_handle;

            Service();

            esp_event_loop_handle_t eventLoop() { return event_loop_handle; }

            void acceptRequest(Protocol* protocol);
            static void parseRequestHeader(uint8_t *header, uint32_t size);

            static void queueTask(void* payload_size_ptr);
            void receiveQueuedRequest(void);
            static void receiveQueuedRequestData(uint8_t *data, uint32_t size);
            static void receiveQueuedRequestData() { receiveQueuedRequestData(NULL, 0); }

            static void onRequestAborted(uint8_t *data, uint32_t bytes_received);
            static void onRequestReceived(uint8_t *data, uint32_t size);
            void onRequestReceived(void) { onRequestReceived(NULL, 0); }

            static void onResponseReady(void *arg, esp_event_base_t base, int32_t id, void *data);
            void sendResponse(void);

            void sendResponseHeader(void);
            static void onResponseHeaderAborted(uint8_t *data, uint32_t size);
            static void onResponseHeaderSent(uint8_t *data, uint32_t size);

            void sendQueuedResponse(void);
            static void sendQueuedResponseData(uint8_t *data, uint32_t size);
            static void sendQueuedResponseData() { sendQueuedResponseData(NULL, 0); }

            void sendStaticResponse(void);

            static void onResponseAborted(uint8_t *data, uint32_t bytes_send);
            static void onResponseSent(uint8_t *data, uint32_t size);

            void finalizeRequest(const char *message, bool success);
    };

}

#endif // WIC64_SERVICE_H
