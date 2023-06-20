#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include <map>
#include <functional>

#include "esp32-hal-log.h"
#include "service.h"

#define WIC64_COMMANDS const std::map<uint8_t, std::function<Command* (Service::Request *request)>> commands
#define WIC64_COMMAND(ID, CLASS) { ID, [](Service::Request* request) { return new CLASS(request); } }

namespace WiC64 {
    class Command {
        private:
            Service::Request* m_request;
            Service::Data* m_emptyResponse;

        public:
            class Undefined;

            static bool defined(uint8_t id);
            static Command* create(Service::Request* request);

            Command(Service::Request* request);
            virtual ~Command();

            Service::Request* request(void) { return m_request; }
            Service::Data* emptyResponse(void) { return m_emptyResponse; }

            virtual Service::Data* execute(void);
    };

    extern WIC64_COMMANDS;
}
#endif // WIC64_COMMAND_H
