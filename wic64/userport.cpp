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
    onTimeoutCallback = onTimeout;
}

bool Userport::isConnected() {
    return connected;
}

void Userport::connect() {
    gpio_set_direction(PA2, GPIO_MODE_INPUT);
    gpio_pullup_en(PA2);

    gpio_set_direction(PC2, GPIO_MODE_INPUT);
    gpio_pulldown_en(PC2);

    gpio_set_direction(FLAG2, GPIO_MODE_OUTPUT);

    setPortToInput();

    attachInterrupt(DATA_DIRECTION_LINE, onDataDirectionChanged, CHANGE);
    connected = true;
}

void Userport::disconnect() {
    gpio_set_direction(PA2, GPIO_MODE_INPUT);
    gpio_pullup_dis(PA2);

    gpio_set_direction(PC2, GPIO_MODE_INPUT);
    gpio_pulldown_dis(PC2);

    gpio_set_direction(FLAG2, GPIO_MODE_INPUT);

    setPortToInput();

    detachInterrupt(DATA_DIRECTION_LINE);
    connected = false;
}

bool Userport::isReadyToSend(void) {
    return isConnected() && isIdle() && IS_LOW(DATA_DIRECTION_LINE);
}

bool Userport::isReadyToReceive() {
    return isConnected() && isIdle() && IS_HIGH(DATA_DIRECTION_LINE);
}

bool Userport::isIdle(void) {
    return !isSending() && !isReceiving();
}

bool Userport::isTransferPending(void) {
    return state == TRANSFER_STATE_PENDING;
}

bool Userport::isSending(void) {
    return type == TRANSFER_TYPE_SEND;
}

bool Userport::isReceiving(void) {
    return type == TRANSFER_TYPE_RECEIVE;
}

void Userport::setTransferRunning() {
    this->state = TRANSFER_STATE_RUNNING;
}

void Userport::setPortToInput() {
    port_config.mode = GPIO_MODE_INPUT;
    gpio_config(&port_config);
    log_d("done");
}

void Userport::setPortToOutput() {
    port_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&port_config);
    log_d("done");
}

void Userport::sendHandshakeSignal() {
    SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);
    ets_delay_us(5);
    SET_LOW(HANDSHAKE_LINE_ESP_TO_C64);
}

void Userport::readNextByte() {
    buffer[pos] = 0;
    for (uint8_t bit=0; bit<8; bit++) {
        if(IS_HIGH(PORT_PIN[bit])) {
            buffer[pos] |= (1<<bit);
        }
    }
    sendHandshakeSignal();

    if (++pos == size) {
        finishTransfer();
    }
}

void Userport::writeNextByte() {
    uint8_t byte = buffer[pos];
    for (uint8_t bit=0; bit<8; bit++) {
        (byte & (1<<bit))
            ? SET_HIGH(PORT_PIN[bit])
            : SET_LOW(PORT_PIN[bit]);
    }
    sendHandshakeSignal();

    if (++pos == size) {
        finishTransfer();
    }
}

void Userport::startTransfer(TRANSFER_TYPE type, uint8_t *data, uint16_t size, void (*onSuccess)(void)) {
    log_d("%s %d bytes...",
        type == TRANSFER_TYPE_SEND ? "Sending" : "Expecting to receive", size);

    this->type = type;
    this->buffer = data;
    this->size = size;
    this->onSuccessCallback = onSuccess;
    this->pos = 0;

    state = (type == TRANSFER_TYPE_SEND)
        ? TRANSFER_STATE_PENDING
        : TRANSFER_STATE_RUNNING;

    attachInterrupt(HANDSHAKE_LINE_C64_TO_ESP, Userport::onHandshakeSignalReceived, RISING);
}

void Userport::finishTransfer(void) {
    log_d("Transfer complete, %d bytes %s",
        pos, type == TRANSFER_TYPE_SEND ? "sent" : "received");

    detachInterrupt(HANDSHAKE_LINE_C64_TO_ESP);
    type = TRANSFER_TYPE_NONE;
    state = TRANSFER_STATE_NONE;

    if (onSuccessCallback != NULL) {
        log_d("Calling onSuccess()...");
        onSuccessCallback();
    }
}

void Userport::send(uint8_t *data, uint16_t size, void (*onSuccess)(void)) {
    startTransfer(TRANSFER_TYPE_SEND, data, size, onSuccess);
}

void Userport::receive(uint8_t *data, uint16_t size, void (*onSuccess)(void)) {
    startTransfer(TRANSFER_TYPE_RECEIVE, data, size, onSuccess);
}

extern Userport *userport;

void Userport::onDataDirectionChanged(void) {
    IS_HIGH(Userport::DATA_DIRECTION_LINE)
        ? userport->setPortToInput()
        : userport->setPortToOutput();

    if (userport->isTransferPending()) {
        log_d("Sending handshake to confirm change of data direction");
        userport->sendHandshakeSignal();
        userport->setTransferRunning();
    }
}

void IRAM_ATTR Userport::onHandshakeSignalReceived(void) {
    userport->isReceiving()
        ? userport->readNextByte()
        : userport->writeNextByte();
}
