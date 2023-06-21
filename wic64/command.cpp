#include "command.h"
#include "commands/commands.h"

namespace WiC64 {

    Command::Command(Request* request) {
        m_request = request;
        m_emptyResponse = new Data(0);
    }

    Command::~Command() {
        delete m_request;
        delete m_emptyResponse;
    }

    bool Command::defined(uint8_t id) {
        return commands.find(id) != commands.end();
    }

    Command* Command::create(Request* request) {
        if (defined(request->id())) {
            return commands.at(request->id())(request);
        }
        return nullptr;
    }

    Data *Command::execute(void) {
        return emptyResponse();
    }
}
