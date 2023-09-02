
#include "update.h"

#include "HTTPUpdate.h"

namespace WiC64 {
    const char* Update::TAG = "UPDATE";

    const char* Update::describe() {
        return "Update (install new firmware)";
    }

    void Update::execute(void) {
        const char* url = request()->argument()->c_str();

        WiFiClient wiFiClient;
        HTTPUpdate httpUpdate;

        ESP_LOGW(TAG, "Installing [%s]", url);

        /* The HTTPUpdate.update() method does not actually flash the new firmware,
         * it simply downloads the specified image, checks if it is a valid ESP32
         * image, writes it to one of the OTA partitions in flash and then
         * immediately resets the ESP.
         *
         * Upon boot, the bootloader then checks for a new firmware image in the
         * OTA partitions and copies it to the actual application partition and then
         * boots the updated application.
         *
         * This means that contrary to the way all other commands work, this command
         * will NOT send a response at all if the update succeeds, since the ESP is
         * already rebooting. A timeout on the client side is thus a sign of success,
         * not failure.
         *
         * Thus any response the client receives is a sign of failure, and the response
         * will contain an appropriate error message.
        */

        switch(httpUpdate.update(wiFiClient, url)) {
            case HTTP_UPDATE_FAILED:
                ESP_LOGE(TAG, "Firmware update failed: %s (%d)",
                    httpUpdate.getLastErrorString().c_str(),
                    httpUpdate.getLastError());
                    response()->copy(httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                ESP_LOGE(TAG, "Firmware update failed: requested version already installed");
                response()->copy("Requested version already installed");
                break;

            case HTTP_UPDATE_OK:
                // This code is never reached since the ESP is already rebooting
                break;
        };
        responseReady();
    }
}
