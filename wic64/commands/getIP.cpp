#include "getIP.h"
#include "connection.h"

namespace WiC64 {

    extern Connection* connection;

    void GetIP::execute(void) {
        response()->wrap(connection->ipAddress());
        responseReady();
    }
}