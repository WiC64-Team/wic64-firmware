#include "echo.h"
#include "service.h"

namespace WiC64 {

Service::Data* Echo::execute(void) {
    return request()->argument();
}

}