#ifdef ARDUINO_ARCH_ESP32
  #include "esp32-hal-log.h"
#endif

#include "display.h"
#include "connection.h"
#include "userport.h"

Display *display;
Connection *connection;
Userport *userport;

#define REQUEST_SIZE 0x27
#define RESPONSE_SIZE 0x02 + 0x04

uint8_t request[REQUEST_SIZE];
uint8_t response[RESPONSE_SIZE] = { 0x00, 0x04, 0xde, 0xad, 0xbe, 0xef };

void responseSent() {
  log_buf_d(response, (size_t) RESPONSE_SIZE);
}

void requestReceived() {
  log_buf_d(request, (size_t) REQUEST_SIZE);
  userport->send(response, RESPONSE_SIZE, responseSent);
}

void setup() {
  display    = new Display();
  connection = new Connection(display);
  userport   = new Userport();
}

void loop() {
  if(userport->isIdle() && userport->isReadyToReceive()) {
    userport->receive(request, REQUEST_SIZE, requestReceived);
  }

  // needed until we are busy enough to avoid the watchdog
  vTaskDelay(1);
}
