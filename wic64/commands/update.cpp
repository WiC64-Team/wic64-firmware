
#include "update.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace WiC64 {
    const char* Update::TAG = "UPDATE";

    const char* Update::describe() {
        return "Update (install new firmware)";
    }

    extern const char wic64_net_pem[] asm("_binary_wic64_net_pem_start");

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    void Update::execute(void) {
        const char* url = request()->argument()->c_str();

        const esp_http_client_config_t config = {
            .url = url,
            .cert_pem = wic64_net_pem,
        };

        ESP_LOGW(TAG, "Installing firmware %s", url);

        if (esp_https_ota(&config) == ESP_OK) {
            response()->copy("0 OK");
        } else {
            response()->copy("1 Firmware update failed");
        }
        responseReady();
    }

    #pragma GCC diagnostic pop
}
