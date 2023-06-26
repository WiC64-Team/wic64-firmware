#include "getIP.h"
#include "connection.h"

namespace WiC64 {

    extern Connection* connection;

    void GetIP::execute(void) {
        response()->data(connection->ipAddress());
        responseReady();
    }
}