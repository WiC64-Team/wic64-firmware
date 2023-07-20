#include "driver/gpio.h"
#include "driver/dac_common.h"
#include "esp32-hal.h"

#include <cmath>

#include "userport.h"
#include "service.h"
#include "utilities.h"

ESP_EVENT_DEFINE_BASE(USERPORT_EVENTS);

namespace WiC64 {
    const char* Userport::TAG = "USERPORT";
    portMUX_TYPE Userport::mutex = portMUX_INITIALIZER_UNLOCKED;

    extern Userport *userport;
    extern Service *service;

    Userport::Userport() {
        esp_event_loop_args_t event_loop_args = {
            .queue_size = 32,
            .task_name = TAG,
            .task_priority = 10,
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
        dac_output_disable(DAC_CHANNEL_1);
        dac_output_disable(DAC_CHANNEL_2);

        gpio_set_direction(PA2, GPIO_MODE_INPUT);
        gpio_set_drive_capability(PA2, GPIO_DRIVE_CAP_3);

        gpio_set_direction(PC2, GPIO_MODE_INPUT);
        gpio_set_drive_capability(PC2, GPIO_DRIVE_CAP_3);

        gpio_set_direction(FLAG2, GPIO_MODE_OUTPUT);
        gpio_set_drive_capability(FLAG2, GPIO_DRIVE_CAP_3);

        setPortToInput();

        gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
        gpio_set_intr_type(HANDSHAKE_LINE_C64_TO_ESP, GPIO_INTR_LOW_LEVEL);
        gpio_isr_handler_add(HANDSHAKE_LINE_C64_TO_ESP, (gpio_isr_t) onHandshakeSignalReceived, NULL);

        connected = true;

        ESP_LOGI(TAG, "Userport connected, accepting requests");
    }

    void Userport::disconnect() {
        gpio_set_direction(PA2, GPIO_MODE_INPUT);

        gpio_set_direction(PC2, GPIO_MODE_INPUT);

        gpio_set_direction(FLAG2, GPIO_MODE_INPUT);

        setPortToInput();

        detachInterrupt(HANDSHAKE_LINE_C64_TO_ESP);
        connected = false;

        ESP_LOGI(TAG, "Userport disconnected");
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
        ESP_LOGV(TAG, "Port set to input");
    }

    void Userport::setPortToOutput() {
        port_config.mode = GPIO_MODE_OUTPUT;
        gpio_config(&port_config);
        ESP_LOGV(TAG, "Port set to output");
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

    inline void IRAM_ATTR Userport::writeFirstByte(void) {
        writeNextByte();
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
            vTaskDelay(pdMS_TO_TICKS(10));
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
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        if (userport->onSuccessCallback != NULL) {
            userport->onSuccessCallback((uint8_t*) userport->buffer, userport->size);
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
            onFailureCallback((uint8_t*) buffer, pos);
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
            return;
        }

        userport->readByte(&api);

        ESP_LOGI(TAG, "Received API id " WIC64_FORMAT_API, api);

        if (service->supports(api)) {
            service->acceptRequest(api);
        } else {
            ESP_LOGE(TAG, "unsupported API id " WIC64_FORMAT_API, api);
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
            return;
        }

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
        portENTER_CRITICAL_ISR(&mutex);

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

        portEXIT_CRITICAL_ISR(&mutex);
        portYIELD_FROM_ISR();
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
