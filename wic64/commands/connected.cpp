
#include "connected.h"
#include "connection.h"

namespace WiC64 {
    const char* Connected::TAG = "CONNECTED";

    extern Connection* connection;

    const char* Connected::describe() {
        return "Connected (test for WiFi connection)";
    }

    void Connected::execute(void) {
        uint8_t s = request()->payload()->data()[0];
        int32_t ms = s * 1000;

        if (!strlen(connection->SSID())) {
            error(CLIENT_ERROR, "WiFi not configured");
            goto DONE;
        }

        while (ms > 0) {
            if (connection->ready()) break;
            vTaskDelay(pdMS_TO_TICKS(100));
            ms -= 100;
        }

        if (!connection->ready()) {
            if (connection->connected()) {
                error(CONNECTION_ERROR, "No IP address assigned");
            }
            else {
                error(CONNECTION_ERROR, "WiFi not connected");
            }
        }
    DONE:
        responseReady();
    }
}
