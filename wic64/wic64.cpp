#include "display.h"
#include "connection.h"
#include "userport.h"
#include "utilities.h"

Display *display;
Connection *connection;
Userport *userport;

void setup() {
    display    = new Display();
    connection = new Connection(display);
    userport   = new Userport();
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10));
}
