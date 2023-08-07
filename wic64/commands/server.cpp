
#include "server.h"
#include "settings.h"

namespace WiC64 {
    const char* Server::TAG = "SERVER";

    extern Settings* settings;

    const char* Server::describe() {
        return "Server (get/set default server)";
    }

    void Server::execute(void) {

        if (request()->id() == 0x08) {
            settings->server(String(
                request()->argument()->data(),
                request()->argument()->size()));
        }

        if (request()->id() == 0x12) {
            response()->copy(settings->server().c_str());
        }

        responseReady();
    }
}
