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
    SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);

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

bool Userport::isReadyToSend(void) {
    return isConnected() && IS_LOW(DATA_DIRECTION_LINE);
}

bool Userport::isReadyToReceive() {
    return isConnected() && IS_HIGH(DATA_DIRECTION_LINE);
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
    SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);
    ets_delay_us(5);
    SET_LOW(HANDSHAKE_LINE_ESP_TO_C64);
}

void Userport::read() {
    buffer[pos] = 0;
    for (uint8_t bit=0; bit<8; bit++) {
        if(IS_HIGH(PORT_PIN[bit])) {
            buffer[pos] |= (1<<bit);
        }
    }
    pos++;
    handshake();
}

void Userport::write() {
    uint8_t byte = buffer[pos];
    for (uint8_t bit=0; bit<8; bit++) {
        (byte & (1<<bit))
            ? SET_HIGH(PORT_PIN[bit])
            : SET_LOW(PORT_PIN[bit]);
    }
    pos++;
    handshake();
}
