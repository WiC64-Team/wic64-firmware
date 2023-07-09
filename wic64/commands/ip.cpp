#include "ip.h"
#include "connection.h"

namespace WiC64 {

    extern Connection* connection;

    const char *IP::describe(void) {
        return "IP (get local IP address)";
    }

    void IP::execute(void) {
        response()->wrap(connection->ipAddress());
        responseReady();
    }
}