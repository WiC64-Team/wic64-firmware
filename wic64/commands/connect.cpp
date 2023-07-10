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

    const char *Connect::passphrase() {
        static const char UPARROW = '~';
        char hexcode[5] = { '0', 'x', 0, 0, 0 };

        const char* escaped = request()->argument()->field(1);

        static char unescaped[MAX_PASSPHRASE_LEN];
        memset(unescaped, 0, MAX_PASSPHRASE_LEN);

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
        const String& ssid = this->ssid();
        const char* passphrase = this->passphrase();

        if (passphrase != NULL) {
            ESP_LOGW(TAG, "Decoded passphrase [%s]", passphrase);
        }
        else if (passphrase == NULL) {
            ESP_LOGE(TAG, "Failed to decode passphrase");
            response()->copy("wifi config not changed: passphrase decode failed");
        }
        else if (ssid.isEmpty()) {
            ESP_LOGE(TAG, "SSID is empty");
            response()->copy("wifi config not changed: ssid empty");
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
