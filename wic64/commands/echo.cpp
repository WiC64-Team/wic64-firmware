#include "echo.h"

namespace WiC64 {
    void Echo::execute(void) {
        response()->data(request()->argument()->data(), request()->argument()->size());
        responseReady();
    }
}