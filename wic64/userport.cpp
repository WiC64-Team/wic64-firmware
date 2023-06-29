#include "driver/gpio.h"
#include "esp32-hal.h"

#include "userport.h"
#include "service.h"
#include "utilities.h"

ESP_EVENT_DEFINE_BASE(USERPORT_EVENTS);

namespace WiC64 {
    const char* Userport::TAG = "USERPORT";

    extern Userport *userport;
    extern Service *service;

    Userport::Userport() {
        esp_event_loop_args_t event_loop_args = {
            .queue_size = 16,
            .task_name = TAG,
            .task_priority = 10,
            .task_stack_size = 8192,
            .task_core_id = 0
        };

        esp_event_loop_create(&event_loop_args, &event_loop_handle);

        esp_event_handler_register_with(
            event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_DATA_DIRECTION_CHANGED,
            onDataDirectionChanged,
            NULL);

        esp_event_handler_register_with(
            event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_HANDSHAKE_SIGNAL_RECEIVED,
            onHandshakeSignalReceived,
            NULL);
    }

    void Userport::connect() {
        gpio_set_direction(PA2, GPIO_MODE_INPUT);
        gpio_pullup_en(PA2);

        gpio_set_direction(PC2, GPIO_MODE_INPUT);
        gpio_pulldown_en(PC2);

        gpio_set_direction(FLAG2, GPIO_MODE_OUTPUT);

        setPortToInput();

        attachInterrupt(DATA_DIRECTION_LINE, dataDirectionChangedISR, CHANGE);
        attachInterrupt(HANDSHAKE_LINE_C64_TO_ESP, handshakeSignalReceivedISR, RISING);
        connected = true;

        ESP_LOGI(TAG, "Userport connected, accepting requests");
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

        ESP_LOGI(TAG, "Userport disconnected");
    }

    bool Userport::isConnected() {
        return connected;
    }

    bool Userport::isReadyToReceiveRequest(void) {
        return IS_HIGH(DATA_DIRECTION_LINE);
    }

    bool Userport::isTransferPending(void) {
        return transferState == TRANSFER_STATE_PENDING;
    }

    void Userport::setTransferRunning() {
        transferState = TRANSFER_STATE_RUNNING;
    }

    bool Userport::isSending(void) {
        return transferType == TRANSFER_TYPE_SEND_PARTIAL ||
            transferType == TRANSFER_TYPE_SEND_FULL;
    }

    bool Userport::isSending(TRANSFER_TYPE type) {
        return type == TRANSFER_TYPE_SEND_PARTIAL ||
            type == TRANSFER_TYPE_SEND_FULL;
    }

    void Userport::setPortToInput() {
        port_config.mode = GPIO_MODE_INPUT;
        gpio_config(&port_config);
        ESP_LOGD(TAG, "Port set to input");
    }

    void Userport::setPortToOutput() {
        port_config.mode = GPIO_MODE_OUTPUT;
        gpio_config(&port_config);
        ESP_LOGD(TAG, "Port set to output");
    }

    inline void Userport::sendHandshakeSignal() {
        SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);
        ets_delay_us(5);
        SET_LOW(HANDSHAKE_LINE_ESP_TO_C64);
    }

    inline void Userport::readByte(uint8_t *byte) {
        (*byte) = 0;
        for (uint8_t bit=0; bit<8; bit++) {
            if(IS_HIGH(PORT_PIN[bit])) {
                (*byte) |= (1<<bit);
            }
        }
    }

    inline void Userport::readNextByte() {
        readByte((uint8_t*) buffer+pos);
        continueTransfer();
    }

    inline void Userport::writeByte(uint8_t *byte) {
        uint8_t value = (*byte);
        for (uint8_t bit=0; bit<8; bit++) {
            (value & (1<<bit))
                ? SET_HIGH(PORT_PIN[bit])
                : SET_LOW(PORT_PIN[bit]);
        }
    }

    inline void Userport::writeNextByte() {
        writeByte((uint8_t*) buffer+pos);
        continueTransfer();
    }

    void Userport::startTransfer(
            TRANSFER_TYPE type,
            uint8_t *data,
            uint16_t size,
            callback_t onSuccess,
            callback_t onFailure) {

        ESP_LOGD(TAG, "%s %d bytes...", isSending(type) ? "Sending" : "Receiving", size);

        this->transferType = type;
        this->onSuccessCallback = onSuccess;
        this->onFailureCallback = onFailure;

        this->buffer = data;
        this->size = size;
        this->pos = 0;

        transferState = isSending()
            ? TRANSFER_STATE_PENDING
            : TRANSFER_STATE_RUNNING;

        createTimeoutTask();

        if (type == TRANSFER_TYPE_RECEIVE_PARTIAL ||
            previousTransferType == TRANSFER_TYPE_RECEIVE_PARTIAL) {
            ESP_LOGV(TAG, "Sending initial handshake signal");
            sendHandshakeSignal();
        }
        timeTransferStarted = millis();
    }

    inline void Userport::continueTransfer(void) {
        if (++pos < size) {
            sendHandshakeSignal();
        } else {
            completeTransfer();
        }
    }

    void Userport::completeTransfer(void) {
        deleteTimeoutTask();

        float sec = (millis() - timeTransferStarted) / 1000.0;
        float kbs = pos/sec/1024;

        ESP_LOGD(TAG, "%d bytes %s, transfer completed in %.4f sec, approx. %.2fkb/s",
            pos, isSending() ? "sent" : "received", sec, kbs);

        TRANSFER_TYPE currentTransferType = transferType;
        previousTransferType = currentTransferType;

        transferState = isSending() ? TRANSFER_STATE_TERMINATING : TRANSFER_STATE_NONE;
        transferType = TRANSFER_TYPE_NONE;
        onFailureCallback = NULL;

        if (currentTransferType != TRANSFER_TYPE_RECEIVE_PARTIAL) {
            ESP_LOGD(TAG, "Sending final handshake signal");
            sendHandshakeSignal();
        }

        if (onSuccessCallback != NULL) {
            onSuccessCallback(buffer, size);
        }
    }

    void Userport::abortTransfer(const char* reason) {
        ESP_LOGW(TAG, "Aborting transfer: %s", reason);
        setPortToInput();

        transferType = TRANSFER_TYPE_NONE;
        previousTransferType = TRANSFER_TYPE_NONE;
        transferState = TRANSFER_STATE_NONE;

        if (onFailureCallback != NULL) {
            onFailureCallback(buffer, pos);
            onFailureCallback = NULL;
        }

        deleteTimeoutTask();
    }

    void Userport::createTimeoutTask(void) {
        xTaskCreatePinnedToCore(timeoutTask, "TIMEOUT", 4096, NULL, 10, &timeoutTaskHandle, 0);

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
        static char reason[32];

        readByte(&api);

        ESP_LOGI(TAG, "Received API id " WIC64_FORMAT_API, api);

        if (!service->supports(api)) {
            // delay required to avoid task timeouts when dealing with line noise
            vTaskDelay(pdMS_TO_TICKS(10));

            snprintf(reason, 32, "unsupported API id " WIC64_FORMAT_API, api);
            abortTransfer(reason);
        }

        service->receiveRequest(api);
    }

    void Userport::receivePartial(uint8_t *data, uint16_t size, callback_t onSuccess) {
        startTransfer(TRANSFER_TYPE_RECEIVE_PARTIAL, data, size, onSuccess, NULL);
    }

    void Userport::receivePartial(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure) {
        startTransfer(TRANSFER_TYPE_RECEIVE_PARTIAL, data, size, onSuccess, onFailure);
    }

    void Userport::receive(uint8_t *data, uint16_t size, callback_t onSuccess) {
        startTransfer(TRANSFER_TYPE_RECEIVE_FULL, data, size, onSuccess, NULL);
    }

    void Userport::receive(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure) {
        startTransfer(TRANSFER_TYPE_RECEIVE_FULL, data, size, onSuccess, onFailure);
    }

    void Userport::sendPartial(uint8_t *data, uint16_t size, callback_t onSuccess) {
        startTransfer(TRANSFER_TYPE_SEND_PARTIAL, data, size, onSuccess, NULL);
    }

    void Userport::sendPartial(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure) {
        startTransfer(TRANSFER_TYPE_SEND_PARTIAL, data, size, onSuccess, onFailure);
    }

    void Userport::send(uint8_t *data, uint16_t size, callback_t onSuccess) {
        startTransfer(TRANSFER_TYPE_SEND_FULL, data, size, onSuccess, NULL);
    }

    void Userport::send(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure) {
        startTransfer(TRANSFER_TYPE_SEND_FULL, data, size, onSuccess, onFailure);
    }

    void IRAM_ATTR Userport::dataDirectionChangedISR(void) {
        esp_event_post_to(
            userport->event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_DATA_DIRECTION_CHANGED,
            NULL, 0, 0);
    }

    void Userport::onDataDirectionChanged(void *arg, esp_event_base_t base, int32_t id, void *data) {
        userport->resetTimeout();

        ESP_LOGD(TAG, "Change of data direction requested");

        IS_HIGH(Userport::DATA_DIRECTION_LINE)
            ? userport->setPortToInput()
            : userport->setPortToOutput();

        if (userport->isTransferPending()) {
            ESP_LOGD(TAG, "Sending handshake signal to start pending transfer");
            userport->sendHandshakeSignal();
            userport->setTransferRunning();
        }
    }

    void IRAM_ATTR Userport::handshakeSignalReceivedISR(void) {
        esp_event_post_to(
            userport->event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_HANDSHAKE_SIGNAL_RECEIVED,
            NULL, 0, 0);
    }

    void Userport::onHandshakeSignalReceived(void *arg, esp_event_base_t base, int32_t id, void *data) {
        userport->resetTimeout();

        if (userport->transferState == TRANSFER_STATE_TERMINATING) {
            userport->transferState = TRANSFER_STATE_NONE;
            ESP_LOGD(TAG, "Received final handshake");
            return;
        }

        switch(userport->transferType) {
            case TRANSFER_TYPE_NONE:
                if (userport->isReadyToReceiveRequest()) {
                    userport->acceptRequest();
                }
                break;

            case TRANSFER_TYPE_RECEIVE_FULL:
            case TRANSFER_TYPE_RECEIVE_PARTIAL:
                userport->readNextByte();
                break;

            case TRANSFER_TYPE_SEND_FULL:
            case TRANSFER_TYPE_SEND_PARTIAL:
                userport->writeNextByte();
                break;
        }
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
}
