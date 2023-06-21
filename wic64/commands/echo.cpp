#include "echo.h"
#include "service.h"

namespace WiC64 {

Data* Echo::execute(void) {
    return request()->argument();
}

}