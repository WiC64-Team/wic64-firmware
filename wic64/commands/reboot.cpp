
#include "reboot.h"
#include "userport.h"
#include "settings.h"

namespace WiC64 {
    const char* Reboot::TAG = "REBOOT";

    extern Settings *settings;
    extern Userport *userport;

    const char* Reboot::describe() {
        return "Reboot (reboot esp32)";
    }

    void Reboot::execute(void) {
        ESP_LOGW(TAG, "Rebooting in 500ms...");
        settings->rebooting(true);
        userport->sendHandshakeSignalBeforeReboot();
        ESP.restart();
    }
}
