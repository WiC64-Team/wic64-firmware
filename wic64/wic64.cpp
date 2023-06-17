#include "display.h"
#include "connection.h"
#include "userport.h"
#include "service.h"
#include "utilities.h"
#include "version.cpp"

#include "esp32-hal-log.h"

Display *display;
Connection *connection;
Service *service;
Userport *userport;

void setup() {
    log_i("WiC64 firmware version %s", WIC64_VERSION_STRING);

    display    = new Display();
    connection = new Connection(display);
    service    = new Service();
    userport   = new Userport(service);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
