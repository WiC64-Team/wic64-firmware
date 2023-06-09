#include "driver/gpio.h"
#include "esp32-hal.h"

#include "userport.h"
#include "service.h"
#include "utilities.h"

extern Userport *userport;
extern Service *service;

Userport::Userport(Service *service) {
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
    attachInterrupt(HANDSHAKE_LINE_C64_TO_ESP, onHandshakeSignalReceived, RISING);
    connected = true;

    log_d("Connected, accepting requests");
}

void Userport::disconnect() {
    gpio_set_direction(PA2, GPIO_MODE_INPUT);
    gpio_pullup_dis(PA2);

    gpio_set_direction(PC2, GPIO_MODE_INPUT);
    gpio_pulldown_dis(PC2);

    gpio_set_direction(FLAG2, GPIO_MODE_INPUT);

    setPortToInput();

    detachInterrupt(DATA_DIRECTION_LINE);
    detachInterrupt(HANDSHAKE_LINE_C64_TO_ESP);
    connected = false;

    log_d("Disconnected");
}

bool Userport::isConnected() {
    return connected;
}

bool Userport::isTransferPending(void) {
    return transferState == TRANSFER_STATE_PENDING;
}

void Userport::setTransferRunning() {
    transferState = TRANSFER_STATE_RUNNING;
}

void Userport::setPortToInput() {
    port_config.mode = GPIO_MODE_INPUT;
    gpio_config(&port_config);
    log_d("Port set to input");
}

void Userport::setPortToOutput() {
    port_config.mode = GPIO_MODE_OUTPUT;
    gpio_config(&port_config);
    log_d("Port set to output");
}

void Userport::sendHandshakeSignal() {
    SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);
    ets_delay_us(5);
    SET_LOW(HANDSHAKE_LINE_ESP_TO_C64);
}

void Userport::readByte(uint8_t *byte) {
    (*byte) = 0;
    for (uint8_t bit=0; bit<8; bit++) {
        if(IS_HIGH(PORT_PIN[bit])) {
            (*byte) |= (1<<bit);
        }
    }
}

void Userport::readNextByte() {
    readByte((uint8_t*) buffer+pos);

    if (++pos < size) {
        sendHandshakeSignal();
    } else {
        completeTransfer();
    }
}

void Userport::writeByte(uint8_t *byte) {
    uint8_t value = (*byte);
    for (uint8_t bit=0; bit<8; bit++) {
        (value & (1<<bit))
            ? SET_HIGH(PORT_PIN[bit])
            : SET_LOW(PORT_PIN[bit]);
    }
}

void Userport::writeNextByte() {
    writeByte((uint8_t*) buffer+pos);

    if (++pos < size) {
        sendHandshakeSignal();
    } else {
        completeTransfer();
    }
}

void Userport::startTransfer(
        TRANSFER_TYPE type,
        uint8_t *data,
        uint16_t size,
        void (*onSuccess)(uint8_t* data, uint16_t size)) {

    log_d("%s %d bytes...",
        type == TRANSFER_TYPE_SEND ? "Sending" : "Receiving", size);

    this->transferType = type;
    this->onSuccessCallback = onSuccess;

    this->buffer = data;
    this->size = size;
    this->pos = 0;

    transferState = (type == TRANSFER_TYPE_SEND)
        ? TRANSFER_STATE_PENDING
        : TRANSFER_STATE_RUNNING;

    createTimeoutTask();

    if (type == TRANSFER_TYPE_SEND ||
        type == TRANSFER_TYPE_RECEIVE_PARTIAL ||
        previousTransferType == TRANSFER_TYPE_RECEIVE_PARTIAL) {
        log_d("Sending initial handshake signal");
        sendHandshakeSignal();
    }
}

void Userport::completeTransfer(void) {
    log_d("Transfer complete, %d bytes %s",
        pos, transferType == TRANSFER_TYPE_SEND ? "sent" : "received");

    deleteTimeoutTask();

    TRANSFER_TYPE currentTransferType = transferType;
    previousTransferType = currentTransferType;

    transferType = TRANSFER_TYPE_NONE;
    transferState = TRANSFER_STATE_NONE;

    onSuccessCallback(buffer, size);

    if (currentTransferType == TRANSFER_TYPE_RECEIVE_FULL ||
        currentTransferType == TRANSFER_TYPE_SEND) {
        log_d("sending final handshake signal");
        sendHandshakeSignal();
    }
}

void Userport::abortTransfer(const char* reason) {
    setPortToInput();

    transferType = TRANSFER_TYPE_NONE;
    previousTransferType = TRANSFER_TYPE_NONE;
    transferState = TRANSFER_STATE_NONE;

    log_d("Transfer aborted: %s", reason);
    deleteTimeoutTask();
}

void Userport::createSessionTask(void) {
    userport->transferType = TRANSFER_TYPE_RECEIVE_REQUEST;
    xTaskCreatePinnedToCore(sessionTask, "SESSION", 8192, NULL, 10, NULL, 1);
}

void Userport::createTimeoutTask(void) {
    xTaskCreatePinnedToCore(timeoutTask, "TIMEOUT", 4096, NULL, 10, &timeoutTaskHandle, 1);

    timeoutTaskCreated = (timeoutTaskHandle != NULL);

    if (!timeoutTaskCreated) {
        abortTransfer("Could not create timeout supervisor task");
    }
}

void Userport::deleteTimeoutTask(void) {
    if (isTimeoutTaskRunning()) {
        vTaskDelete(timeoutTaskHandle);
        timeoutTaskCreated = false;
    }
}

bool Userport::isTimeoutTaskRunning(void) {
    return
        (timeoutTaskHandle != NULL) &&                  // Task has been created successfully before
        (timeoutTaskCreated) &&                         // Task has been created and should be running
        (eTaskGetState(timeoutTaskHandle) != eDeleted); // Task has not previously been deleted
}

void Userport::resetTimeout(void) {
    timeOfLastActivity = millis();
}

bool Userport::hasTimedOut(void) {
    return millis() - timeOfLastActivity > timeout;
}

void Userport::acceptRequest(void) {
    uint8_t api;

    log_d("Accepting request...");
    readByte(&api);

    log_d("Received API id 0x%02X (%c)", api, api);

    if (!service->supports(api)) {
        abortTransfer("Unknown API id requested");
    }

    service->receiveRequest(api);
}

void Userport::receivePartial(uint8_t *data, uint16_t size, void (*onSuccess)(uint8_t* data, uint16_t size)) {
    startTransfer(TRANSFER_TYPE_RECEIVE_PARTIAL, data, size, onSuccess);
}

void Userport::receive(uint8_t *data, uint16_t size, void (*onSuccess)(uint8_t* data, uint16_t size)) {
    startTransfer(TRANSFER_TYPE_RECEIVE_FULL, data, size, onSuccess);
}

void Userport::send(uint8_t *data, uint16_t size, void (*onSuccess)(uint8_t* data, uint16_t size)) {
    startTransfer(TRANSFER_TYPE_SEND, data, size, onSuccess);
}

void Userport::onDataDirectionChanged(void) {
    userport->resetTimeout();

    log_d("Change of data direction requested");

    IS_HIGH(Userport::DATA_DIRECTION_LINE)
        ? userport->setPortToInput()
        : userport->setPortToOutput();

    if (userport->isTransferPending()) {
        log_d("Sending handshake signal to resume pending transfer");
        userport->sendHandshakeSignal();
        userport->setTransferRunning();
    }
}

void IRAM_ATTR Userport::onHandshakeSignalReceived(void) {
    userport->resetTimeout();

    switch(userport->transferType) {
        case TRANSFER_TYPE_NONE:
            userport->createSessionTask();
            break;

        case TRANSFER_TYPE_RECEIVE_FULL:
        case TRANSFER_TYPE_RECEIVE_PARTIAL:
            userport->readNextByte();
            break;

        case TRANSFER_TYPE_SEND:
            userport->writeNextByte();
            break;

        case TRANSFER_TYPE_RECEIVE_REQUEST:
            break;
    }
}

void Userport::sessionTask(void *) {
    userport->acceptRequest();
    vTaskDelete(NULL);
}

void Userport::timeoutTask(void* unused) {
    userport->resetTimeout();

    while(true) {
        vTaskDelay(pdMS_TO_TICKS(100));

        if (userport->hasTimedOut()) {
            userport->abortTransfer("Timed out");
        }
    }
}
