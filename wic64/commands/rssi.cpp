#include "rssi.h"
#include "connection.h"

namespace WiC64 {
    const char* RSSI::TAG = "RSSI";

    extern Connection *connection;

    const char* RSSI::describe() {
        return "RSSI (get RSSI)";
    }

    void RSSI::execute(void) {
        response()->copyString(connection->RSSI());
        responseReady();
    }
}
