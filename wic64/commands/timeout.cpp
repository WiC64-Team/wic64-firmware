
#include "timeout.h"

namespace WiC64 {
    const char* Timeout::TAG = "TIMEOUT";

    const char* Timeout::describe() {
        return "Timeout (set ESP transfer timeout)";
    }

    void Timeout::execute(void) {
        Data* payload = request()->payload();
        uint8_t seconds;

        if (payload->size() < 1) {
            ESP_LOGE(TAG, "No ESP timeout specified in payload");
            error(CLIENT_ERROR, "No ESP timeout in payload");
            goto DONE;
        }

        seconds = payload->data()[0];

        if (seconds == 0) {
            ESP_LOGE(TAG, "ESP timeout must be at least 1 second");
            error(CLIENT_ERROR, "ESP timeout must be >= 1 second");
            goto DONE;
        }

        customTimeout = seconds * 1000;

        ESP_LOGI(TAG, "ESP timeout set to %dms", timeout);

    DONE:
        responseReady();
    }
}
