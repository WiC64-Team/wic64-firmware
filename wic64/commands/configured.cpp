
#include "configured.h"
#include "connection.h"

namespace WiC64 {
    const char* Configured::TAG = "CONFIGURED";

    extern Connection* connection;

    const char* Configured::describe() {
        return "Configured (Check if WiFi credentials present)";
    }

    bool Configured::supportsProtocol(void) {
        return request()->protocol()->id() == Protocol::STANDARD;
    }

    void Configured::execute(void) {
        if (!connection->configured()) {
            error(Command::INTERNAL_ERROR, "WiFi not configured");
        }
        responseReady();
    }
}
