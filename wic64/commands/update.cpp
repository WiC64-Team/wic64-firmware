
#include "update.h"
#include "utilities.h"
#include "led.h"

#include "esp_http_client.h"
#include "esp_https_ota.h"

namespace WiC64 {
    const char* Update::TAG = "UPDATE";

    extern Led* led;

    const char* Update::describe() {
        return "Update (install new firmware)";
    }

    extern const char wic64_net_pem[] asm("_binary_wic64_net_pem_start");

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    void Update::execute(void) {
        const char* url = request()->argument()->c_str();
        static char buffer[256];
        char* message = buffer;
        esp_err_t result;

        const esp_http_client_config_t config = {
            .url = url,
            .cert_pem = wic64_net_pem,
        };

        ESP_LOGW(TAG, "Installing firmware %s", url);
        led->on();

        if ((result = esp_https_ota(&config)) == ESP_OK) {
            success("OK");
        } else {
            ESP_LOGE(TAG, "Firmware update failed: %s", esp_err_to_name(result));
            esp_err_to_name_r(result, message, 256);
            replace(message, '_', '-');
            error(INTERNAL_ERROR, message);
        }

        led->off();
        responseReady();
    }

    #pragma GCC diagnostic pop
}
