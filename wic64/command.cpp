#include "command.h"
#include "commands/commands.h"

namespace WiC64 {

    Command::Command(Service::Request* request) {
        m_request = request;
        m_emptyResponse = new Service::Data(0);
    }

    Command::~Command() {
        delete m_request;
        delete m_emptyResponse;
    }

    bool Command::defined(uint8_t id) {
        return commands.find(id) != commands.end();
    }

    Command* Command::create(Service::Request* request) {
        if (defined(request->id())) {
            return commands.at(request->id())(request);
        }
        return nullptr;
    }

    Service::Data *Command::execute(void) {
        return emptyResponse();
    }
}
