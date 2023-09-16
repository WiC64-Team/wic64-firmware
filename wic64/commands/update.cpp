
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
        httpUpdate.rebootOnUpdate(false);

        ESP_LOGW(TAG, "Installing [%s]", url);

        switch(httpUpdate.update(wiFiClient, url)) {
            case HTTP_UPDATE_FAILED:
                ESP_LOGE(TAG, "Firmware update failed: %s (%d)",
                    httpUpdate.getLastErrorString().c_str(),
                    httpUpdate.getLastError());
                    response()->appendField("1");
                    response()->appendField(httpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                ESP_LOGE(TAG, "Firmware update failed: requested version already installed");
                response()->appendField("2");
                response()->appendField("Requested version already installed");
                break;

            case HTTP_UPDATE_OK:
                response()->appendField("0");
                response()->appendField("OK");
                break;
        };
        responseReady();
    }
}
