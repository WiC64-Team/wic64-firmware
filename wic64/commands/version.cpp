
#include "version.h"

namespace WiC64 {
    const char* Version::TAG = "VERSION";

    const char* Version::describe() {
        return "Version (get firmware version)";
    }

    void Version::execute(void) {
        static const uint8_t version[3] = {
            WIC64_VERSION_MAJOR,
            WIC64_VERSION_MINOR,
            WIC64_VERSION_PATCH,
        };

        if (request()->id() == 0x00) {
            response()->copy(WIC64_VERSION_STRING);
        }
        if (request()->id() == 0x26) {
            response()->wrap((uint8_t*) version, 3);
        }
        responseReady();
    }
}
