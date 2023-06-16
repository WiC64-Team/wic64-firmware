#include "display.h"
#include "connection.h"
#include "userport.h"
#include "service.h"
#include "utilities.h"

Display *display;
Connection *connection;
Service *service;
Userport *userport;

void setup() {
    display    = new Display();
    connection = new Connection(display);
    service    = new Service();
    userport   = new Userport(service);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
