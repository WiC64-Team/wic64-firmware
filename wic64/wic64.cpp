#ifdef ARDUINO_ARCH_ESP32
  #include "esp32-hal-log.h"
#endif

#include "display.h"
#include "connection.h"
#include "userport.h"

Display *display;
Connection *connection;
Userport *userport;

void setup() {
  display    = new Display();
  connection = new Connection(display);
  userport   = new Userport();
}

void loop() {
  // needed until we are busy enough to avoid the watchdog
  vTaskDelay(pdMS_TO_TICKS(1000));
}
