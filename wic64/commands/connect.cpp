#include "connection.h"
#include "connect.h"
#include "commands.h"

#include "WiFi.h"

namespace WiC64 {
    const char* Connect::TAG = "CONNECT";

    extern Connection *connection;

    const char *Connect::describe() {
        return "Connect (connect to WiFi with specified credentials)";
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
            strncpy(ssid, ssid_string.c_str(), ssid_string.length());
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
            error(CLIENT_ERROR, "wifi config not changed: passphrase decode failed");
        }
        else if (strlen(ssid) == 0) {
            ESP_LOGE(TAG, "SSID is empty");
            error(CLIENT_ERROR, "wifi config not changed: ssid empty");
        }
        else {
            connection->connect(ssid, passphrase);
            success("wifi config changed");

            // After receiving the response, the portal's wifi.prg
            // (and probably launcher.prg) immediately try to determine
            // whether a connection has been established by requesting
            // the ip nine times in quick succession before returning to
            // password entry, printing "Not connected. Wrong password?".
            //
            // The previous firmware added a delay of 3 seconds so that
            // the client would not start checking the IP right away.
            //
            // REDESIGN: This is not the firmwares job, add a command
            // to explicitly check connection state with optional timeout.
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        responseReady();
    }
}
