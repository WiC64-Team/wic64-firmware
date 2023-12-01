
#include "hardware.h"

namespace WiC64 {
    const char* Hardware::TAG = "HARDWARE";

    const char* Hardware::describe() {
        return "Hardware (test if running on real hardware)";
    }

    void Hardware::execute(void) {
        //error(INTERNAL_ERROR, "WiC64 is emulated");
        responseReady();
    }
}
