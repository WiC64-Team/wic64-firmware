#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include <map>
#include <functional>

#include "wic64.h"
#include "service.h"

#define WIC64_COMMANDS const std::map<uint8_t, std::function<Command* (Request *request)>> commands
#define WIC64_COMMAND(ID, CLASS) { ID, [](Request* request) { return new CLASS(request); } }

namespace WiC64 {
    class Command {
        private:
            Request* m_request;
            Data* m_emptyResponse;

        public:
            class Undefined;

            static bool defined(uint8_t id);
            static Command* create(Request* request);

            Command(Request* request);
            virtual ~Command();

            bool isVersion1() { return m_request->api() == WiC64::API_V1; }
            bool isVersion2() { return m_request->api() == WiC64::API_V2; }

            Request* request(void) { return m_request; }
            Data* emptyResponse(void) { return m_emptyResponse; }

            virtual Data* execute(void);
    };

    extern WIC64_COMMANDS;
}
#endif // WIC64_COMMAND_H
