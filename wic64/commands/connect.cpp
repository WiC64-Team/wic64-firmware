#include "connection.h"
#include "connect.h"

#include "WiFi.h"

namespace WiC64 {
    const char* Connect::TAG = "CONNECT";

    extern Connection *connection;

    const char *Connect::describe() {
        return "Connect (connect to WiFi with specified credentials)";
    }

    void Connect::execute(void)
    {
        uint8_t indexInLastScanResult = atoi(request()->argument()->field(0));
        const char* password = request()->argument()->field(1);

        const String& ssid = WiFi.SSID(indexInLastScanResult).c_str();

        connection->connect(ssid.c_str(), password);

        response()->copy("Wlan config changed");
        responseReady();
    }
}
