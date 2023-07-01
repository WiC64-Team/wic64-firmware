#include "main.h"

using namespace WiC64;

void setup() {
    new WiC64::WiC64();
}

void loop() {
    webserver->serve();
    vTaskDelay(pdMS_TO_TICKS(100));
}
