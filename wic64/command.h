#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include <functional>

#include "wic64.h"
#include "service.h"

#define WIC64_COMMANDS command_map_entry_t commands[]
#define WIC64_COMMAND(ID, CLASS) { .id = ID, .create = [](Request* request) { return new CLASS(request); } }

namespace WiC64 {
    class Command {
        private:
            Request* m_request = NULL;
            Data* m_response = NULL;
            bool m_response_ready = false;

        public:
            const static char* TAG;
            class Undefined;

            static bool defined(uint8_t id);
            static Command* create(Request* request);

            Command(Request* request);
            virtual ~Command();

            bool isVersion1() { return m_request->api() == WiC64::API_V1; }
            bool isVersion2() { return m_request->api() == WiC64::API_V2; }

            Request* request(void) { return m_request; }
            Data* response(void) { return m_response; }
            bool isResponseReady() { return m_response_ready; }

            virtual void execute(void);
            virtual void responseReady();
    };

    struct command_map_entry_t {
        uint8_t id;
        std::function<Command* (Request *request)> create;
    };

    extern WIC64_COMMANDS;
}
#endif // WIC64_COMMAND_H
