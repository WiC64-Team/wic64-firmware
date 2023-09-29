#include "echo.h"

namespace WiC64 {
    const char *Echo::describe() {
        return "Echo (respond with request data)";
    }

    void Echo::execute(void) {
        response()->set(request()->argument());
        responseReady();
    }
}