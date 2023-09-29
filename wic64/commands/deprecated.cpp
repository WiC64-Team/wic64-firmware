#include "deprecated.h"
#include "commands.h"

namespace WiC64 {
    const char* Deprecated::TAG = "DEPRECATED";

    const char* Deprecated::describe() {
        return "Deprecated command (since 2.0.0)";
    }

    const char *Deprecated::reason(void) {
        static char reason[512+1];
        uint16_t available = sizeof(reason);

        uint16_t prefix_size = snprintf(reason, available,
            "Command $%02x has been deprecated since firmware version 2.0.0:\r\n", id());

        uint16_t remaining = available - prefix_size;
        char* cursor = reason + prefix_size;

        switch(id()) {
            case WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_03:
            case WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_04:
            case WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_05:
            case WIC64_CMD_DEPRECATED_FIRMWARE_UPDATE_REQUIRED_18:
                snprintf(cursor, remaining, "%s",
                    "Firmware updates are now handled by the new update command $27. "
                    "The old update prorgram(s) may still appear to be working, but will "
                    "actually no longer install any updates.");
                break;

            case WIC64_CMD_DEPRECATED_GET_STATS_07:
                snprintf(cursor, remaining, "%s",
                    "Firmware is now versioned semantically, use command $00 to get "
                    "a version string to display to the user, or use command $26 to get "
                    "the version as individual bytes (<major> <minor> <patch> <devel>)");
                break;

            case WIC64_CMD_DEPRECATED_LOG_TO_SERIAL_CONSOLE_09:
                snprintf(cursor, remaining, "%s",
                    "We see no reason to log to the esp console from the c64. "
                    "If you *really* need this, please drop us a line.");
                break;

            case WIC64_CMD_DEPRECATED_GET_UPD_0A:
            case WIC64_CMD_DEPRECATED_SEND_UPD_0B:
            case WIC64_CMD_DEPRECATED_SET_UPD_PORT_0E:
            case WIC64_CMD_DEPRECATED_GET_UPD_DUPLICATE_1E:
            case WIC64_CMD_DEPRECATED_SEND_UPD_DUPLICATE_1F:
                snprintf(cursor, remaining, "%s",
                    "The UPD commands have always been marked as \"work in progress\" and "
                    "have never been used by any programms as far as we are aware of. We may "
                    "implement them properly in the future, should the need arise.");
                break;

            case WIC64_CMD_DEPRECATED_GET_EXTERNAL_IP_13:
                snprintf(cursor, remaining, "%s",
                    "The command for getting the external IP has never worked "
                    "correctly, nor has it been used in any programs we are aware of.");
                break;

            case WIC64_CMD_DEPRECATED_GET_PREFERENCES_19:
            case WIC64_CMD_DEPRECATED_SET_PREFERENCES_1A:
                snprintf(cursor, remaining, "%s",
                    "The commands for getting or setting ESP preferences from the c64 "
                    "have been removed because they are inherently unsafe. Third parties "
                    "should not be able to write arbitrary data to the ESP flash memory.");
                break;

            case WIC64_CMD_DEPRECATED_SET_TCP_PORT_20:
                snprintf(cursor, remaining, "%s",
                    "This command was called \"set tcp port\" but has apparently never "
                    "had any effect in the legacy firmware, apart from setting a global "
                    "variable that was never used.");
                break;

            case WIC64_CMD_DEPRECATED_LEGACY_HTTP_POST_24:
                snprintf(cursor, remaining, "%s",
                    "The previous implementation of HTTP POST was broken and the "
                    "payload structure was unnecessarily complex. Please use the "
                    "new HTTP POST command $28 instead.");
                break;

            case WIC64_CMD_DEPRECATED_BIG_LOADER_25:
                snprintf(cursor, remaining, "%s",
                    "This command was called \"big loader\" and was intended as a"
                    "HTTP GET request implementation for transfer sizes above 64kb. "
                    "It has never worked properly and has never been officially "
                    "documented. General support for payloads larger than 64kb will "
                    "be added in the future.");
                break;

            case WIC64_CMD_DEPRECATED_FACTORY_RESET_63:
                snprintf(cursor, remaining, "%s",
                    "The factory reset command has been deprecated. The web interface "
                    "will allow clearing all preferences instead.");
                break;

            default:
                snprintf(cursor, remaining, "%s",
                    "Deprecated for unknown reasons");
        }
        return reason;
    }

    void Deprecated::execute(void) {
        ESP_LOGE(TAG, "%s", reason());
        response()->copyString(reason());
        responseReady();
    }
}
