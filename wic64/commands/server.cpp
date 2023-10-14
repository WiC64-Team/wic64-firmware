
#include "server.h"
#include "commands.h"
#include "settings.h"

namespace WiC64 {
    const char* Server::TAG = "SERVER";

    extern Settings* settings;

    const char* Server::describe() {
        return "Server (get/set default server)";
    }

    void Server::execute(void) {

        if (id() == WIC64_CMD_SET_SERVER) {
            settings->server(String(
                request()->payload()->data(),
                request()->payload()->size()));
        }

        if (id() == WIC64_CMD_GET_SERVER) {
            response()->copyString(settings->server().c_str());
        }

        responseReady();
    }
}
