
#include "configured.h"
#include "connection.h"
#include "settings.h"

namespace WiC64 {
    const char* Configured::TAG = "CONFIGURED";

    extern Connection* connection;
    extern Settings* settings;

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
        else {
            response()->appendByte(settings->passphraseLength());
        }
        responseReady();
    }
}
