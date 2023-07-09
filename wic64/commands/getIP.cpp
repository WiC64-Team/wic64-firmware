#include "getIP.h"
#include "connection.h"

namespace WiC64 {

    extern Connection* connection;

    const char *GetIP::describe(void) {
        return "IP (get local IP address)";
    }

    void GetIP::execute(void)
    {
        response()->wrap(connection->ipAddress());
        responseReady();
    }
}