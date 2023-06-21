#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wic64.h"

void setup() {
    new WiC64::WiC64();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
