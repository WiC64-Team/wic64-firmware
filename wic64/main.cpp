#include "main.h"

using namespace WiC64;

void setup() {
    new WiC64::WiC64();
}

void loop() {
    webserver->serve();
    buttons->tick();
    vTaskDelay(pdMS_TO_TICKS(10));
}
