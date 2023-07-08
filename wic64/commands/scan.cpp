#include "scan.h"
#include "utilities.h"

#include "WiFi.h"

namespace WiC64 {

    const char* Scan::TAG = "SCAN";

    void Scan::execute(void) {
        int num_networks;

        ESP_LOGD(TAG, "Scanning for WiFi networks...");

        num_networks = WiFi.scanNetworks();
        num_networks = MIN(num_networks, 15);

        for(uint8_t i=0; i<num_networks; i++) {

            ESP_LOGD(TAG, "Network %d: [%s] %ddbm",
                i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));

            response()->appendSeparated(String(i));
            response()->appendSeparated(WiFi.SSID(i));
            response()->appendSeparated(String(WiFi.RSSI(i)));
        }
        responseReady();
    }
}
