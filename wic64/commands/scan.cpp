#include "scan.h"
#include "connection.h"
#include "utilities.h"

#include "WiFi.h"

namespace WiC64 {
    const char* Scan::TAG = "SCAN";

    extern Connection *connection;

    void Scan::execute(void) {
        int num_networks;

        ESP_LOGI(TAG, "Scanning for WiFi networks...");

        num_networks = connection->scanNetworks();
        num_networks = MIN(num_networks, 15);

        // The previous firmware only checked for num_networks == 0
        // and returned "no networks found". We also test <= 0
        // and issue the appropriate log message, but we still just
        // return "no networks found", which remains true in either
        // case, although it's not entirely accurate.

        if (num_networks <= 0 ) {
            if (num_networks < 0) {
                ESP_LOGE(TAG, "Scan failed with error code %d", num_networks);
            }
            else {
                ESP_LOGW(TAG, "Scan successful, still 0 networks found");
            }
            response()->copy("no networks found");
        }
        else {
            ESP_LOGI(TAG, "Scan successful, %d networks found", num_networks);

            for(uint8_t i=0; i<num_networks; i++) {

                ESP_LOGD(TAG, "Network %d: [%s] %ddbm",
                    i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));

                response()->appendField(String(i));
                response()->appendField(WiFi.SSID(i));
                response()->appendField(String(WiFi.RSSI(i)));
            }
        }
        responseReady();
    }
}
