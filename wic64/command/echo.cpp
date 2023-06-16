#include "echo.h"
#include "service.h"

Service::Data* Echo::execute(void) {
    return request()->argument();
}