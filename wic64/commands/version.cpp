
#include "version.h"
#include "commands.h"

namespace WiC64 {
    const char* Version::TAG = "VERSION";

    const char* Version::describe() {
        return "Version (get firmware version)";
    }

    void Version::execute(void) {
        static uint8_t version[4] = {
            WIC64_VERSION_MAJOR,
            WIC64_VERSION_MINOR,
            WIC64_VERSION_PATCH,
            WIC64_VERSION_DEVEL,
        };

        if (id() == WIC64_CMD_VERSION_STRING) {
            response()->copyString(WIC64_VERSION_STRING);
        }

        if (id() == WIC64_CMD_VERSION_NUMBERS) {
            response()->set((uint8_t*) version, 4);
        }

        responseReady();
    }
}
