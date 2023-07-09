#include "connection.h"
#include "connect.h"

#include "WiFi.h"

namespace WiC64 {
    const char* Connect::TAG = "CONNECT";

    extern Connection *connection;

    const char *Connect::describe() {
        return "Connect (connect to WiFi with specified credentials)";
    }

    const String Connect::ssid() {
        if (request()->id() == SSID_PASSED_AS_STRING) {
            return String(request()->argument()->field(0));
        }
        else if (request()->id() == SSID_PASSED_VIA_INDEX) {
            uint8_t indexInLastScan = atoi(request()->argument()->field(0));
            return String(WiFi.SSID(indexInLastScan));
        }
        return String();
    }

    void Connect::execute(void) {
        const String& ssid = this->ssid();
        const char* passphrase = request()->argument()->field(1);

        if (ssid.isEmpty()) {
            ESP_LOGE(TAG, "SSID is empty");
            response()->copy("wifi config not changed");
        }
        else {
            connection->connect(ssid.c_str(), passphrase);
            response()->copy("wifi config changed");

            // After receiving the response, the portal's wifi.prg
            // (and probably launcher.prg) immediately try to determine
            // whether a connection has been established by requesting
            // the ip nine times in quick succession before returning to
            // password entry, printing "Not connected. Wrong password?".
            //
            // The previous firmware added a delay of 3 seconds so that
            // the client would not start checking the IP right away.
            //
            // REDESIGN: This is not the firmware's job, add a command
            // to explicitly check connection state with optional timeout.
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        responseReady();
    }
}
