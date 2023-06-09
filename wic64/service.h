#ifndef WIC64_SERVICE_H
#define WIC64_SERVICE_H

#include <cstdint>
#include "esp_event.h"

#ifdef ___cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(SERVICE_EVENTS);

enum {
    SERVICE_REQUEST_RECEIVED_EVENT
};

#ifdef ___cplusplus
}
#endif

class Service {
    public:
        class Command;

    private:
        static const uint8_t API_V1_ID = 'W';
        static const uint8_t API_V1_COMMAND_SIZE = 4;
        static const uint8_t HEADER_SIZE = 3;

        Command *command;

        static void parseRequestHeaderV1(uint8_t *header, uint16_t size);

    public:
        esp_event_loop_handle_t event_loop_handle;

        Service();
        bool supports(uint8_t apiId);

        void receiveRequest(uint8_t apiId);
        static void postRequestReceivedEvent(uint8_t* ignoreData, uint16_t ignoreSize);
        static void onRequestReceived(void* arg, esp_event_base_t base, int32_t id, void* data);

        void sendResponse(uint8_t *data, uint16_t size);
        static void onResponseSent(uint8_t *data, uint16_t size);

        class Data {
            private:
                uint8_t *_data;
                uint16_t _size;

            public:
                Data(uint16_t size);
                ~Data();

                uint8_t* data() { return _data; }
                uint16_t size() { return _size; }
        };

        class Command {
            private:
                uint8_t _id;
                uint8_t _argc;
                Data** _argv;

                int16_t getNextFreeArgumentIndex();

            public:
                Command(uint8_t api, uint8_t id, uint8_t argc);
                ~Command();

                uint8_t id(void) { return _id; };
                uint8_t argc(void) { return _argc; };

                Data* addArgument(Data* argument);
                Data* argument(uint8_t index);
        };
};

#endif // WIC64_SERVICE_H