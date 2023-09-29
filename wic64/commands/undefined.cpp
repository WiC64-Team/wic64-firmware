
#include "undefined.h"

namespace WiC64 {
    const char* Undefined::TAG = "UNDEFINED";

    const char* Undefined::describe() {
        return "Undefined command";
    }

    void Undefined::execute(void) {
        static char message[32];
        snprintf(message, sizeof(message), "Undefined command id 0x%02x", id());
        ESP_LOGE(TAG, "%s", message);
        response()->copyString(message);
        responseReady();
    }
}
