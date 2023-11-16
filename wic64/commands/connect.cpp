#include "connect.h"
#include "commands.h"
#include "connection.h"
#include "display.h"

#include "WiFi.h"

namespace WiC64 {
    const char* Connect::TAG = "CONNECT";

    extern Connection *connection;
    extern Display *display;

    const char *Connect::describe() {
        switch (id()) {
            case WIC64_CMD_CONNECT_WITH_SSID_STRING:
                return "Connect (connect with SSID and pass))";

            case WIC64_CMD_CONNECT_WITH_SSID_INDEX:
                return "Connect (connect witth SSID scan index and pass)";

            default: return "Unknown Connect command";
        }
    }

    const char* Connect::ssid() {
        static char ssid[MAX_SSID_LEN+1];
        static char indexAsString[3];
        uint8_t indexInLastScan;
        String ssid_string;

        if (id() == WIC64_CMD_CONNECT_WITH_SSID_STRING) {
            request()->payload()->field(0, ssid);
        }
        else if (id() == WIC64_CMD_CONNECT_WITH_SSID_INDEX) {
            request()->payload()->field(0, indexAsString);
            indexInLastScan = atoi(indexAsString);
            ssid_string = WiFi.SSID(indexInLastScan);
            strncpy(ssid, ssid_string.c_str(), ssid_string.length()+1);
        }
        return ssid;
    }

    const char *Connect::passphrase() {
        static const char UPARROW = '~';
        static char hexcode[5] = { '0', 'x', 0, 0, 0 };
        static char escaped[MAX_PASSPHRASE_LEN+1];
        static char unescaped[MAX_PASSPHRASE_LEN+1];

        request()->payload()->field(1, escaped);
        memset(unescaped, 0, MAX_PASSPHRASE_LEN+1);

        uint8_t len = strlen(escaped);
        char* pos = unescaped;

        char *error[] = { NULL };
        char decoded;

        for (uint8_t i=0; i<len; i++) {
            if (escaped[i] == UPARROW) {
                hexcode[2] = escaped[++i];
                hexcode[3] = escaped[++i];

                decoded = (char) strtol(hexcode, error, 16);

                if (**error != '\0') {
                    ESP_LOGE(TAG, "Failed to decode hex value [%s]", hexcode+2);
                    return NULL;
                }

                *pos = decoded;
            }
            else {
                *pos = escaped[i];
            }

            if (((++pos) - unescaped) > (MAX_PASSPHRASE_LEN - 1)) {
                ESP_LOGE(TAG, "Passphrase too long: max 63 chars");
                return NULL;
            }
        }
        return unescaped;
    }

    void Connect::execute(void) {
        const char* ssid = this->ssid();
        const char* passphrase = this->passphrase();

        if (passphrase == NULL) {
            ESP_LOGE(TAG, "Failed to decode passphrase");
            error(CLIENT_ERROR, "passphrase decode failed");
        }
        else if (strlen(ssid) == 0) {
            ESP_LOGE(TAG, "SSID is empty");
            error(CLIENT_ERROR, "ssid empty");
        }
        else {
            connection->connect(ssid, passphrase);
            display->connectionConfigured(true);
            success("wifi config changed");

            // A 3000ms delay is required since WiFi.begin() uses interrupts and
            // blocks other tasks for a short time. If a new request comes in
            // during this time, the irq watchdog times out. This delay forces
            // the client to wait at least three seconds before sending the next
            // request.
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        responseReady();
    }
}
