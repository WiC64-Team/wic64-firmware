#include "command.h"
#include "command/echo.h"

#define WIC64_COMMAND_ECHO 0xff

Command* Command::create(Service::Request* request) {
    switch (request->id()) {
        case WIC64_COMMAND_ECHO:
            return new Echo(request);

        default:
            return new Command::Undefined(request);
    }
}

Command::Command(Service::Request* request) : m_request{ request }  { }
Command::~Command() { delete m_request; }
