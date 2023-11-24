#include "test.h"
#include "commands.h"

namespace WiC64 {
    const char *Test::describe() {
        switch (id()) {
            case WIC64_CMD_ECHO:
                return "Echo (respond with request data)";
                break;

            case WIC64_CMD_FORCE_TIMEOUT:
                return "Force timeout (delay to force client timeout)";
                break;

            case WIC64_CMD_FORCE_ERROR:
                return "Force error (return INTERNAL_ERROR and test error message)";
                break;

            default:
                return "Unknown Test command";
                break;
        }
    }

    void Test::execute(void) {
        if (id() == WIC64_CMD_ECHO) {
            response()->set(request()->payload());
        }

        else if (id() == WIC64_CMD_FORCE_TIMEOUT) {
            Data* payload = request()->payload();

            uint8_t s = payload->size()
                ? request()->payload()->data()[0]
                : 1;

            uint32_t ms = s * 1000;
            vTaskDelay(pdMS_TO_TICKS(ms));
        }

        else if (id() == WIC64_CMD_FORCE_ERROR) {
            error(INTERNAL_ERROR, "Test error");
        }

        responseReady();
    }
}