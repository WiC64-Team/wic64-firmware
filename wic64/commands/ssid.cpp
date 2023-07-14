#include "ssid.h"
#include "connection.h"

namespace WiC64 {
    const char* SSID::TAG = "SSID";

    extern Connection *connection;

    const char* SSID::describe() {
        return "SSID (get SSID)";
    }

    void SSID::execute(void) {
        response()->wrap(connection->SSID());
        responseReady();
    }
}
