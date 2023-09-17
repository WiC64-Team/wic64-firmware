#include "esp32-hal.h"
#include "esp_event.h"

#include "wic64.h"
#include "service.h"
#include "userport.h"
#include "data.h"
#include "command.h"
#include "display.h"
#include "utilities.h"

ESP_EVENT_DEFINE_BASE(SERVICE_EVENTS);

namespace WiC64 {
    const char* Service::TAG = "SERVICE";

    extern Service *service;
    extern Userport *userport;
    extern Display *display;

    Service::Service() {
        esp_event_loop_args_t event_loop_args = {
            .queue_size = 16,
            .task_name = TAG,
            .task_priority = 15,
            .task_stack_size = 8192,
            .task_core_id = 1
        };

        esp_event_loop_create(&event_loop_args, &event_loop_handle);

        esp_event_handler_register_with(
            event_loop_handle,
            SERVICE_EVENTS,
            SERVICE_RESPONSE_READY,
            onResponseReady,
            NULL);

        ESP_LOGI(TAG, "Command service initialized");
    }

    bool Service::supports(uint8_t api) {
        return api == WiC64::API_V1;
    }

    void Service::acceptRequest(uint8_t api) {
        static uint8_t header[REQUEST_HEADER_SIZE];

        if (api == WiC64::API_V1) {
            userport->receivePartial(header, REQUEST_HEADER_SIZE, parseRequestHeaderVersion1);
        }
    }

    void Service::parseRequestHeaderVersion1(uint8_t *header, uint16_t size) {
        ESP_LOGD(TAG, "Parsing api version 1 request header...");
        ESP_LOG_HEXV(TAG, "Header", (uint8_t*) header, size);

        uint8_t api = WiC64::API_V1;
        uint8_t id = header[2];

        uint16_t argument_size = (*((uint16_t*) header)) - API_V1_ARGUMENT_SIZE_CORRECTION;
        bool has_argument = (argument_size > 0);

        service->request = new Request(api, id, has_argument ? 1 : 0);

        ESP_LOGI(TAG, "Received request header "
                      WIC64_CYAN("[0x%02x 0x%02x ") WIC64_FORMAT_CMD WIC64_CYAN("] ")
                      WIC64_GREEN("(payload %d bytes)"),
            header[0],
            header[1],
            service->request->id(),
            argument_size);

        if (service->request->hasArguments()) {
            ESP_LOGI(TAG, "Receiving request argument");
            Data* argument = service->request->addArgument(new Data(transferBuffer, argument_size));
            userport->receive(argument->data(), argument->size(), onRequestReceived, onRequestAborted);
        }
        else {
            service->onRequestReceived();
        }
    }

    void Service::onRequestAborted(uint8_t *data, uint16_t bytes_received) {
        ESP_LOGW(TAG, "Received %d bytes of %d bytes",
            bytes_received, service->request->argument(0)->size());

        ESP_LOGW(TAG, "Aborted while receiving request");
        ESP_LOGW(TAG, "Freeing allocated memory");

        delete service->request;
        service->request = NULL;
    }

    void Service::onRequestReceived(uint8_t *ignoredData, uint16_t ignoredSize) {
        Request *request = service->request;

        ESP_LOGI(TAG, "Request received successfully");
        ESP_LOGI(TAG, "Handling request [" WIC64_FORMAT_CMD WIC64_GREEN("]"), request->id());
        ESP_LOGV(TAG, "Request arguments...");

        for (uint8_t i=0; i<request->argc(); i++) {
            Data* argument = request->argument(i);
            static char title[12+1];
            snprintf(title, 12, "argument %hhu", i);
            ESP_LOG_HEXV(TAG, title, argument->data(), argument->size());
        }

        service->command = Command::create(request);

        ESP_LOGI(TAG, "Executing command " WIC64_WHITE("") "%s", service->command->describe());
        service->command->execute();
    }

    void Service::onResponseReady(void *arg, esp_event_base_t base, int32_t id, void *data) {
        service->sendResponse();
    }

    void Service::sendResponse() {
        sendResponseHeader();
    }

    void Service::sendResponseHeader() {
        response = command->response();

        // For unknown reasons the response size is transferred in
        // big-endian format in API version 1 (high byte first)
        static uint8_t responseSizeBuffer[2];

        responseSizeBuffer[0] = HIGHBYTE(response->sizeToReport());
        responseSizeBuffer[1] = LOWBYTE(response->sizeToReport());

        ESP_LOGI(TAG, "Sending response size %d [0x%02x, 0x%02x]",
            response->sizeToReport(),
            responseSizeBuffer[0],
            responseSizeBuffer[1]);

        response->isPresent()
            ? userport->sendPartial(responseSizeBuffer, 2, onResponseHeaderSent, onResponseHeaderAborted)
            : userport->send(responseSizeBuffer, 2, onResponseHeaderSent, onResponseHeaderAborted);
    }

    void Service::onResponseHeaderAborted(uint8_t *data, uint16_t bytes_sent) {
        ESP_LOGW(TAG, "Sent %d of 2 bytes", bytes_sent);
        service->finalizeRequest("Aborted while sending response size", false);
    }

    void Service::onResponseHeaderSent(uint8_t* data, uint16_t size) {
        Data *response = service->response;

        response->isEmpty()
            ? service->finalizeRequest("Request handled successfully", true)
            : response->isQueued()
                ? service->sendQueuedResponse()
                : service->sendStaticResponse();
    }

    void Service::sendQueuedResponse() {
        Data *response = command->response();

        ESP_LOGD(TAG, "Preparing queued send of %d bytes", response->size());

        bytes_remaining = response->size();
        items_remaining = WIC64_QUEUE_ITEMS_REQUIRED(response->size());

        service->sendQueuedResponseData();
    }

    void Service::sendQueuedResponseData(uint8_t *isSubsequentCall, uint16_t ignoreSize) {
        static uint8_t data[WIC64_QUEUE_ITEM_SIZE];
        Data *response = service->command->response();

        vTaskDelay(10);

        if (isSubsequentCall != NULL) {
            service->items_remaining--;
        }

        ESP_LOGV(TAG, "%s call of sendQueuedResponseData(), %d bytes in %d item%s remaining",
            (isSubsequentCall == NULL) ? "First" : "Subsequent",
            service->bytes_remaining,
            service->items_remaining,
            (service->items_remaining > 1) ? "s" : "");

        uint16_t size = (service->items_remaining > 1)
            ? WIC64_QUEUE_ITEM_SIZE
            : service->bytes_remaining;

        const uint16_t timeout_ms = 1000;
        uint8_t attempts = 8;

        while (xQueueReceive(response->queue(), data, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
            ESP_LOGW(TAG, "Could not read next item from queue in %dms, attempting %d more times",
                timeout_ms, attempts-1);

            if(--attempts == 0) {
                onResponseAborted(NULL, service->bytes_remaining);
                return;
            }
        }
        service->bytes_remaining -= size;

        (service->items_remaining > 1)
            ? userport->sendPartial((uint8_t*) data, size, sendQueuedResponseData, onResponseAborted)
            : userport->send((uint8_t*) data, size, onResponseSent, onResponseAborted);
    }

    void Service::sendStaticResponse(void) {
        userport->send(response->data(), response->size(), onResponseSent, onResponseAborted);
    }

    void Service::onResponseAborted(uint8_t *data, uint16_t bytes_sent)
    {
        Data *response = service->command->response();
        ESP_LOGW(TAG, "Sent only %d of %d bytes", bytes_sent, response->size());

        service->finalizeRequest("Aborted while sending response", false);
    }

    void Service::onResponseSent(uint8_t *data, uint16_t size) {
        ESP_LOG_HEXV(TAG, "response", data, size);
        service->finalizeRequest("Request handled successfully", true);
    }

    void Service::finalizeRequest(const char* message, bool success) {
        esp_log_level_t level;

        level = success ? ESP_LOG_INFO : ESP_LOG_WARN;
        ESP_LOG_LEVEL(level, TAG, "Finalizing request: %s", message);

        if (command != NULL) {
            if(command->response()->isQueued()) {
                ESP_LOG_LEVEL(ESP_LOG_DEBUG, TAG, "Resetting response queue");
                xQueueReset(command->response()->queue());
            }

            level = success ? ESP_LOG_DEBUG : ESP_LOG_WARN;
            ESP_LOG_LEVEL(level, TAG, "Freeing allocated memory");

            delete command;
            command = NULL;
        }
        else {
            ESP_LOGE(TAG, "Request has already been finalized: "
                "protocol violation or firmware bug");
        }

        vTaskDelay(pdMS_TO_TICKS(10));

        log_free_mem(TAG, ESP_LOG_VERBOSE);
        display->update();
    }
}
