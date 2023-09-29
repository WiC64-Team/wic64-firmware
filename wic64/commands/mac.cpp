#include "mac.h"
#include "connection.h"

namespace WiC64 {
    const char* MAC::TAG = "MAC";

    extern Connection *connection;

    const char* MAC::describe() {
        return "MAC (get MAC address)";
    }

    void MAC::execute(void) {
        response()->copyString(connection->macAddress());
        responseReady();
    }
}
