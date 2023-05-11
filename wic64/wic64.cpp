#include "connection.h"
#include "userport.h"
#include "display.h"

#ifdef ARDUINO_ARCH_ESP32
  #include "esp32-hal-log.h"
#endif

Display *display;
Connection *connection;
Userport *userport;

void setup() { 
    display = new Display();    
    connection = new Connection();
    userport = new Userport();
}
void loop() {
    // needed until we are busy enough to avoid the watchdog to bite...
    vTaskDelay(1); 
 }