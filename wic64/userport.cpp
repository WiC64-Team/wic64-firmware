#include "driver/gpio.h"
#include "esp32-hal.h"

#include <cmath>

#include "userport.h"
#include "service.h"
#include "led.h"
#include "utilities.h"

ESP_EVENT_DEFINE_BASE(USERPORT_EVENTS);

namespace WiC64 {
    const char* Userport::TAG = "USERPORT";
    portMUX_TYPE Userport::mutex = portMUX_INITIALIZER_UNLOCKED;

    extern Userport *userport;
    extern Service *service;
    extern Led *led;

    Userport::Userport() {
        gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

        esp_event_loop_args_t event_loop_args = {
            .queue_size = 1,
            .task_name = TAG,
            .task_priority = 15,
            .task_stack_size = 8192,
            .task_core_id = 0
        };

        esp_event_loop_create(&event_loop_args, &event_loop_handle);

        esp_event_handler_register_with(
            event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_REQUEST_INITIATED,
            onRequestInitiated,
            NULL);

        esp_event_handler_register_with(
            event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_READY_TO_SEND,
            onReadyToSend,
            NULL);

        esp_event_handler_register_with(
            event_loop_handle,
            USERPORT_EVENTS,
            USERPORT_TRANSFER_COMPLETED,
            onTransferCompleted,
            NULL);
    }

    void Userport::connect() {
        gpio_set_direction(HANDSHAKE_LINE_ESP_TO_C64, GPIO_MODE_OUTPUT);

        gpio_set_direction(HANDSHAKE_LINE_C64_TO_ESP, GPIO_MODE_INPUT);
        gpio_set_intr_type(HANDSHAKE_LINE_C64_TO_ESP, GPIO_INTR_LOW_LEVEL);
        gpio_isr_handler_add(HANDSHAKE_LINE_C64_TO_ESP, (gpio_isr_t) onHandshakeSignalReceived, NULL);

        gpio_set_direction(DATA_DIRECTION_LINE, GPIO_MODE_INPUT);
        gpio_set_intr_type(DATA_DIRECTION_LINE, GPIO_INTR_POSEDGE);
        gpio_isr_handler_add(DATA_DIRECTION_LINE, (gpio_isr_t) onDataDirectionChanged, NULL);

        setPortToInput();

        connected = true;

        ESP_LOGI(TAG, "Userport connected, accepting requests");
    }

    void Userport::disconnect() {
        gpio_set_direction(HANDSHAKE_LINE_ESP_TO_C64, GPIO_MODE_INPUT);

        gpio_set_direction(HANDSHAKE_LINE_C64_TO_ESP, GPIO_MODE_INPUT);
        gpio_set_intr_type(HANDSHAKE_LINE_C64_TO_ESP, GPIO_INTR_DISABLE);
        gpio_isr_handler_remove(HANDSHAKE_LINE_C64_TO_ESP);

        gpio_set_direction(DATA_DIRECTION_LINE, GPIO_MODE_INPUT);
        gpio_set_intr_type(DATA_DIRECTION_LINE, GPIO_INTR_DISABLE);
        gpio_isr_handler_remove(DATA_DIRECTION_LINE);

        setPortToInput();

        connected = false;
        ESP_LOGI(TAG, "Userport disconnected, ignoring requests");
    }

    bool Userport::isConnected() {
        return connected;
    }

    bool Userport::isReadyToReceive(void) {
        return IS_HIGH(DATA_DIRECTION_LINE);
    }

    bool Userport::isReadyToSend(void) {
        return IS_LOW(DATA_DIRECTION_LINE);
    }

    bool Userport::isTransferPending(void) {
        return transferState == TRANSFER_STATE_PENDING;
    }

    void Userport::setTransferRunning() {
        transferState = TRANSFER_STATE_RUNNING;
    }

    bool Userport::isInitiallySending(void) {
        return (transferType == TRANSFER_TYPE_SEND_FULL ||
                transferType == TRANSFER_TYPE_SEND_PARTIAL) &&
                previousTransferType != TRANSFER_TYPE_SEND_PARTIAL;
    }

    bool Userport::isSending(void) {
        return isSending(transferType);
    }

    bool Userport::isSending(TRANSFER_TYPE type) {
        return type == TRANSFER_TYPE_SEND_PARTIAL ||
            type == TRANSFER_TYPE_SEND_FULL;
    }

    void Userport::setPortToInput() {
        port_config.mode = GPIO_MODE_INPUT;
        gpio_config(&port_config);
        led->off();
        ESP_LOGV(TAG, "Port set to input");
    }

    void Userport::setPortToOutput() {
        port_config.mode = GPIO_MODE_OUTPUT;
        gpio_config(&port_config);
        led->on();
        ESP_LOGV(TAG, "Port set to output");
    }

    inline void Userport::sendHandshakeSignal() {
        SET_HIGH(HANDSHAKE_LINE_ESP_TO_C64);
        SET_LOW(HANDSHAKE_LINE_ESP_TO_C64);
    }

    inline void Userport::readByte(uint8_t *byte) {
        (*byte) = 0;
        IS_HIGH(PB0) && ((*byte) |= 1);
        IS_HIGH(PB1) && ((*byte) |= 2);
        IS_HIGH(PB2) && ((*byte) |= 4);
        IS_HIGH(PB3) && ((*byte) |= 8);
        IS_HIGH(PB4) && ((*byte) |= 16);
        IS_HIGH(PB5) && ((*byte) |= 32);
        IS_HIGH(PB6) && ((*byte) |= 64);
        IS_HIGH(PB7) && ((*byte) |= 128);
    }

    inline void Userport::readNextByte() {
        readByte(buffer+pos);
        continueTransfer();
    }

    inline void Userport::writeByte(uint8_t *byte) {
        register uint8_t value = (*byte);
        (value & 1)   ? SET_HIGH(PB0) : SET_LOW(PB0);
        (value & 2)   ? SET_HIGH(PB1) : SET_LOW(PB1);
        (value & 4)   ? SET_HIGH(PB2) : SET_LOW(PB2);
        (value & 8)   ? SET_HIGH(PB3) : SET_LOW(PB3);
        (value & 16)  ? SET_HIGH(PB4) : SET_LOW(PB4);
        (value & 32)  ? SET_HIGH(PB5) : SET_LOW(PB5);
        (value & 64)  ? SET_HIGH(PB6) : SET_LOW(PB6);
        (value & 128) ? SET_HIGH(PB7) : SET_LOW(PB7);
    }

    inline void IRAM_ATTR Userport::writeFirstByte() {
        writeNextByte();
    }

    inline void Userport::writeNextByte() {
        writeByte(buffer+pos);
        continueTransfer();
    }

    void Userport::startTransfer(
            TRANSFER_TYPE type,
            uint8_t *data,
            uint16_t size,
            callback_t onSuccess,
            callback_t onFailure) {

        this->transferType = type;
        this->onSuccessCallback = onSuccess;
        this->onFailureCallback = onFailure;

        this->buffer = data;
        this->size = size;
        this->pos = 0;

        transferState = isInitiallySending()
            ? TRANSFER_STATE_PENDING
            : TRANSFER_STATE_RUNNING;

        ESP_LOGD(TAG, "%s %d bytes...", isSending(type) ? "Sending" : "Receiving", size);

        createTimeoutTask();
        timeTransferStarted = millis();

        if (isInitiallySending()) {
            ESP_LOGV(TAG, "Sending initial handshake to start pending transfer");
            sendHandshakeSignal(); // first handshake the c64 is waiting for after changing direction
        }

        if (type == TRANSFER_TYPE_RECEIVE_PARTIAL ||
            previousTransferType == TRANSFER_TYPE_RECEIVE_PARTIAL ||
            previousTransferType == TRANSFER_TYPE_SEND_PARTIAL) {

            ESP_LOGV(TAG, "Sending initial handshake signal");
            vTaskDelay(pdMS_TO_TICKS(10));
            sendHandshakeSignal();
        }
    }

    inline void Userport::continueTransfer(void) {
        if (++pos < size) {
            sendHandshakeSignal();
        } else {
            post(USERPORT_TRANSFER_COMPLETED);
        }
    }

    void Userport::onTransferCompleted(void* arg, esp_event_base_t base, int32_t id, void* data) {
        userport->deleteTimeoutTask();

        float sec = (millis() - userport->timeTransferStarted) / 1000.0;
        float kbs = userport->size/sec/1024;

        if (kbs != INFINITY) {
            ESP_LOGI(TAG, "%d bytes %s, transfer completed in %.4f sec, approx. %.2fkb/s",
            userport->size, userport->isSending() ? "sent" : "received", sec, kbs);
        } else {
            ESP_LOGI(TAG, "%d bytes %s, transfer completed",
                userport->size, userport->isSending() ? "sent" : "received");
        }
        vTaskDelay(pdMS_TO_TICKS(1));

        TRANSFER_TYPE currentTransferType = userport->transferType;
        userport->previousTransferType = currentTransferType;

        userport->transferState = userport->isSending()
            ? TRANSFER_STATE_TERMINATING
            : TRANSFER_STATE_NONE;

        userport->transferType = TRANSFER_TYPE_NONE;
        userport->onFailureCallback = NULL;

        if (currentTransferType == TRANSFER_TYPE_RECEIVE_FULL ||
            currentTransferType == TRANSFER_TYPE_SEND_FULL) {
            ESP_LOGD(TAG, "Sending final handshake signal");
            userport->sendHandshakeSignal();
        }

        if (userport->onSuccessCallback != NULL) {
            userport->onSuccessCallback(userport->buffer, userport->size);
        }

        if (currentTransferType != TRANSFER_TYPE_SEND_PARTIAL) {
            userport->setPortToInput();
        }
    }

    void Userport::abortTransfer(const char* reason) {
        ESP_LOGE(TAG, "Aborting transfer: %s", reason);
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
        xTaskCreatePinnedToCore(timeoutTask, "TIMEOUT", 4096, NULL, 5, &timeoutTaskHandle, 0);

        if (timeoutTaskHandle == NULL) {
            abortTransfer("Could not create timeout supervisor task");
        }
    }

    void Userport::deleteTimeoutTask(void) {
        if (timeoutTaskHandle != NULL) {
            portENTER_CRITICAL(&mutex);

            vTaskDelete(timeoutTaskHandle);
            timeoutTaskHandle = NULL;

            portEXIT_CRITICAL(&mutex);
        }
    }

    void Userport::resetTimeout(void) {
        timeOfLastActivity = millis();
    }

    bool Userport::hasTimedOut(void) {
        return millis() - timeOfLastActivity > timeout;
    }

    void Userport::onRequestInitiated(void* arg, esp_event_base_t base, int32_t id, void* data) {
        uint8_t api;

        ESP_LOGV(TAG, "Received initial handshake");

        if (!userport->isReadyToReceive()) {
            ESP_LOGE(TAG, "Userport not ready to receive - PA2 is not HIGH");
            userport->handleLineNoise();
            return;
        }

        userport->readByte(&api);

        ESP_LOGI(TAG, "Received API id " WIC64_FORMAT_API, api);

        if (service->supports(api)) {
            userport->resetLineNoiseCount();
            service->acceptRequest(api);

        } else {
            ESP_LOGE(TAG, "Unsupported API id " WIC64_FORMAT_API, api);
            userport->handleLineNoise();
        }
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

    void IRAM_ATTR Userport::onReadyToSend(void *arg, esp_event_base_t base, int32_t id, void *data) {
        ESP_LOGV(TAG, "Received handshake after change of transfer direction");

        if (!userport->isReadyToSend()) {
            ESP_LOGE(TAG, "Userport not ready to send - PA2 is still HIGH");
            userport->handleLineNoise();
            return;
        }
        userport->resetLineNoiseCount();
        userport->setPortToOutput();
        userport->setTransferRunning();

        ESP_LOGV(TAG, "Initiating send by writing first byte");
        userport->writeFirstByte();
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

    void Userport::onHandshakeSignalReceived(void) {
        userport->resetTimeout();

        if (userport->transferState == TRANSFER_STATE_TERMINATING) {
            userport->transferState = TRANSFER_STATE_NONE;
        }

        else if (userport->transferState == TRANSFER_STATE_PENDING) {
            userport->post(USERPORT_READY_TO_SEND);
        }

        else if (userport->transferType == TRANSFER_TYPE_NONE) {
            userport->post(USERPORT_REQUEST_INITIATED);
        }

        else if (userport->transferType == TRANSFER_TYPE_RECEIVE_FULL ||
            userport->transferType == TRANSFER_TYPE_RECEIVE_PARTIAL) {
            userport->readNextByte();
        }
        else if (userport->transferType == TRANSFER_TYPE_SEND_FULL ||
            userport->transferType == TRANSFER_TYPE_SEND_PARTIAL) {
            userport->writeNextByte();
        }
    }

    void IRAM_ATTR Userport::onDataDirectionChanged(void) {
        userport->transferType = TRANSFER_TYPE_NONE;
    }

    void Userport::resetLineNoiseCount(void) {
        lineNoiseCount = 0;
    }

    void Userport::handleLineNoise(void) {
        if (++lineNoiseCount >= 8) {
            if (transferType != TRANSFER_TYPE_NONE) {
                abortTransfer("Line noise detected");
            }

            ESP_LOGW(TAG, "Line noise detected => disconnecting userport for 500ms");

            userport->disconnect();
            vTaskDelay(pdMS_TO_TICKS(500));
            userport->connect();

            resetLineNoiseCount();
        }
    }

    void IRAM_ATTR Userport::post(userport_event_t event) {
        BaseType_t task_unblocked;

        esp_event_isr_post_to(
            userport->event_loop_handle,
            USERPORT_EVENTS, event,
            NULL, 0,
            &task_unblocked);

        if (task_unblocked) {
            portYIELD_FROM_ISR();
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
