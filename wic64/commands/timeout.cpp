
#include "timeout.h"
#include "commands.h"

namespace WiC64 {
    const char* Timeout::TAG = "TIMEOUT";

    const char* Timeout::describe() {
        switch (id()) {
            case WIC64_CMD_SET_TRANSFER_TIMEOUT:
                return "Timeout (set transfer timeout)";
                break;
            case WIC64_CMD_SET_REMOTE_TIMEOUT:
                return "Timeout (set remote request timeout)";
                break;
            default:
                return "Unhandled Timeout command id (?)";
        }
    }

    void Timeout::execute(void) {
        Data* payload = request()->payload();
        uint8_t seconds;

        const char *type = id() == WIC64_CMD_SET_TRANSFER_TIMEOUT
            ? "transfer"
            : "HTTP request";

        if (payload->size() < 1) {
            ESP_LOGE(TAG, "No %s timeout specified in payload", type);
            error(CLIENT_ERROR, "Timeout value not specified");
            goto DONE;
        }

        seconds = payload->data()[0];

        if (seconds == 0) {
            ESP_LOGE(TAG, "%s timeout must be at least 1 second", type);
            error(CLIENT_ERROR, "Timeout must be >= 1 second");
            goto DONE;
        }

        if (id() == WIC64_CMD_SET_TRANSFER_TIMEOUT) {
            customTransferTimeout = seconds * 1000;
            ESP_LOGI(TAG, "Transfer timeout set to %dms", customTransferTimeout);
        }
        else {
            customRemoteTimeout = seconds * 1000;
            ESP_LOGI(TAG, "HTTP request timeout set to %dms", customRemoteTimeout);
        }

    DONE:
        responseReady();
    }
}
