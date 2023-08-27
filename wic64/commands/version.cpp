
#include "version.h"
#include "commands.h"

namespace WiC64 {
    const char* Version::TAG = "VERSION";

    const char* Version::describe() {
        return "Version (get firmware version)";
    }

    void Version::execute(void) {
        static uint8_t version[3] = {
            WIC64_VERSION_MAJOR,
            WIC64_VERSION_MINOR,
            WIC64_VERSION_PATCH,
        };

        if (id() == WIC64_CMD_VERSION_STRING) {
            response()->copy(WIC64_VERSION_STRING);
        }

        if (id() == WIC64_CMD_VERSION_NUMBERS) {
            response()->wrap((uint8_t*) version, 3);
        }

        responseReady();
    }
}
