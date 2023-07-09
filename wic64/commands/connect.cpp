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

        // After receiving the response, the portal's wifi.prg
        // (and probably launcher.prg) immediately try to determine
        // whether a connection has been established by requesting
        // the ip nine times in quick succession before giving up
        // and printing "Not connected. Wrong password?"
        // The previous firmware added a delay of 3 seconds so that
        // the client would not start checking the IP right away.

        // REDESIGN: This is not the firmware's job
        vTaskDelay(pdMS_TO_TICKS(3000));

        responseReady();
    }
}
