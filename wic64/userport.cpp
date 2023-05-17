#include "userport.h"
#include "driver/gpio.h"
#include "esp32-hal.h"

Userport::Userport() {
    connect();
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

bool Userport::isConnected() {
    return connected;
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
    state = TRANSFER_STATE_RUNNING;
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
        completeTransfer();
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
        completeTransfer();
    }
}

void Userport::startTransfer(TRANSFER_TYPE type, uint8_t *data, uint16_t size, void (*onSuccess)(void), uint16_t timeout) {
    log_d("%s %d bytes...",
        type == TRANSFER_TYPE_SEND ? "Sending" : "Expecting to receive", size);

    this->timeout = timeout;
    this->type = type;
    this->buffer = data;
    this->size = size;
    this->onSuccessCallback = onSuccess;
    this->pos = 0;

    state = (type == TRANSFER_TYPE_SEND)
        ? TRANSFER_STATE_PENDING
        : TRANSFER_STATE_RUNNING;

    attachInterrupt(HANDSHAKE_LINE_C64_TO_ESP, Userport::onHandshakeSignalReceived, RISING);

    if (timeout > 0) {
        createTimeoutTask();
    }
}

void Userport::completeTransfer(void) {
    log_d("Transfer complete, %d bytes %s",
        pos, type == TRANSFER_TYPE_SEND ? "sent" : "received");

    deleteTimeoutTask();
    detachInterrupt(HANDSHAKE_LINE_C64_TO_ESP);

    type = TRANSFER_TYPE_NONE;
    state = TRANSFER_STATE_NONE;

    if (onSuccessCallback != NULL) {
        log_d("Calling onSuccess()...");
        onSuccessCallback();
    }
}

void Userport::abortTransfer(const char* reason) {
    detachInterrupt(HANDSHAKE_LINE_C64_TO_ESP);
    setPortToInput();

    type = TRANSFER_TYPE_NONE;
    state = TRANSFER_STATE_NONE;

    log_d("Transfer aborted: %s", reason);

    deleteTimeoutTask();
}

void Userport::createTimeoutTask(void) {
    resetTimeout();
    timeoutTaskHandle = NULL;

    log_d("Creating timeout supervisor task");
    xTaskCreatePinnedToCore(timeoutTask, "Timeout", 4096, NULL, 10, &timeoutTaskHandle, 1);

    if (timeoutTaskHandle == NULL) {
        abortTransfer("Could not create timeout supervisor task");
    }
}

void Userport::deleteTimeoutTask(void) {
    if(isTimeoutTaskRunning()) {
        log_d("Deleting timeout supervisor task");
        vTaskDelete(timeoutTaskHandle);
    }
}

bool Userport::isTimeoutTaskRunning(void) {
    return timeoutTaskHandle != NULL;
}

void Userport::resetTimeout(void) {
    timeOfLastActivity = millis();
}

bool Userport::hasTimedOut(void) {
    return millis() - timeOfLastActivity > timeout;
}

void Userport::send(uint8_t *data, uint16_t size, void (*onSuccess)(void)) {
    startTransfer(TRANSFER_TYPE_SEND, data, size, onSuccess, DEFAULT_TIMEOUT);
}

void Userport::send(uint8_t *data, uint16_t size, void (*onSuccess)(void), uint16_t timeout) {
    startTransfer(TRANSFER_TYPE_SEND, data, size, onSuccess, timeout);
}

void Userport::receive(uint8_t *data, uint16_t size, void (*onSuccess)(void), uint16_t timeout) {
    startTransfer(TRANSFER_TYPE_RECEIVE, data, size, onSuccess, timeout);
}

void Userport::receive(uint8_t *data, uint16_t size, void (*onSuccess)(void)) {
    startTransfer(TRANSFER_TYPE_RECEIVE, data, size, onSuccess, DEFAULT_TIMEOUT);
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

    userport->resetTimeout();
}

void IRAM_ATTR Userport::onHandshakeSignalReceived(void) {
    userport->isReceiving()
        ? userport->readNextByte()
        : userport->writeNextByte();

    userport->resetTimeout();
}

void Userport::timeoutTask(void* unused) {
    while(true) {
        vTaskDelay(pdMS_TO_TICKS(100));

        if (userport->hasTimedOut()) {
            userport->abortTransfer("Timed out");
        }
    }
}
