#include "echo.h"

namespace WiC64 {
    const char *Echo::describe() {
        return "Echo (respond with request data)";
    }

    void Echo::execute(void) {
        response()->wrap(request()->argument()->data(), request()->argument()->size());
        responseReady();
    }
}