#ifndef WIC64_SERVICE_H
#define WIC64_SERVICE_H

#include <cstdint>

#define LOWBYTE(UINT16) (uint8_t) ((UINT16 >> 0UL) & 0xff)
#define HIGHBYTE(UINT16) (uint8_t) ((UINT16 >> 8UL) & 0xff)

namespace WiC64 {

class Command;
class Service {
    public:
        class Request;
        class Data;

    private:
        static const uint8_t API_V1_ID = 'W';
        static const uint8_t API_V1_REQUEST_SIZE = 4;
        static const uint8_t REQUEST_HEADER_SIZE = 3;

        uint8_t* responseData;
        uint16_t responseSize;

        Request *request;
        Command *command;
        Data *response;

        void deleteCommand(void);
        static void parseRequestHeaderV1(uint8_t *header, uint16_t size);

    public:
        Service() { };
        bool supports(uint8_t apiId);

        void receiveRequest(uint8_t apiId);
        static void onRequestReceived(uint8_t* data, uint16_t size);

        void sendResponse();
        static void onResponseSizeSent(uint8_t *data, uint16_t size);
        static void onResponseSent(uint8_t *data, uint16_t size);

        class Data {
            private:
                uint8_t *m_data;
                uint16_t m_size;

            public:
                Data(uint16_t size);
                ~Data();

                bool isPresent(void);
                bool isEmpty(void);

                uint8_t* data() { return m_data; }
                uint16_t size() { return m_size; }
        };

        class Request {
            private:
                uint8_t m_id;
                uint8_t m_argc;
                Data** m_argv;

                int16_t getNextFreeArgumentIndex();

            public:
                Request(uint8_t api, uint8_t id, uint8_t argc);
                ~Request();

                uint8_t id(void) { return m_id; };
                uint8_t argc(void) { return m_argc; };

                Data* addArgument(Data* argument);
                Data* argument(void);
                Data* argument(uint8_t index);
        };
};

}

#endif // WIC64_SERVICE_H
