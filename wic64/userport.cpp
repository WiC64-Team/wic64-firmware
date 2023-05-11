#include "userport.h"
#include "driver/gpio.h"
#include "esp32-hal.h"

Userport::Userport() {
    connect();
}

void Userport::setTimeout(uint16_t ms) {
    timeout = ms;
}

void Userport::onTimeout(void (* onTimeout)()) {
    timeoutHandler = onTimeout;
}

bool Userport::isConnected() {
    return connected;
}

void Userport::connect() {
    pc2_and_pa2_config.mode = GPIO_MODE_INPUT;
    gpio_config(&pc2_and_pa2_config);

    flag2_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&flag2_config);

    input();
    connected = true;
}

void Userport::disconnect() {
    pc2_and_pa2_config.mode = GPIO_MODE_INPUT;
    gpio_config(&pc2_and_pa2_config);

    flag2_config.mode = GPIO_MODE_INPUT;
    gpio_config(&flag2_config);

    input();
    connected = false;
}

bool Userport::isReady() {
    if (!isConnected()) return false;
    return ((GPIO.in >> PA2) & 1); // TODO: && notInTransfer
}

void Userport::input() {
    port_config.mode = GPIO_MODE_INPUT;
    gpio_config(&port_config);
}

void Userport::output() {
    port_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&port_config);
}

void Userport::handshake() {
    GPIO.out_w1ts = (1 << FLAG2);
    delayMicroseconds(5);
    GPIO.out_w1tc = (1 << FLAG2);
}

void Userport::read() { // PC2 irq from peer?
    buffer[pos] = 0;
    for (uint8_t i=0; i<8; i++) {
        if((GPIO.in >> PORT_PIN[i]) & 1) {
            buffer[pos] |= (1<<i);
        }
    }
    pos++;
    handshake();
}

void Userport::write() { // PC2 irq from peer?
    uint8_t byte = buffer[pos];
    for (uint8_t i=0; i<8; i++) {
        (byte & (1<<i))
            ? GPIO.out_w1ts = (1<<PORT_PIN[i])
            : GPIO.out_w1tc = (1<<PORT_PIN[i]);
    }
    pos++;
    handshake();
}
