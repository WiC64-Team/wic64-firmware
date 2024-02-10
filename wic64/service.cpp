#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp32-hal.h"
#include "esp_event.h"

#include "wic64.h"
#include "service.h"
#include "protocol.h"
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

    void Service::receiveRequest(Protocol *protocol) {
        receiveRequestHeader(protocol);
    }

    void Service::receiveRequestHeader(Protocol *protocol) {
        static uint8_t header[Protocol::MAX_REQUEST_HEADER_SIZE];
        this->protocol = protocol;

        if (command != NULL) {
            ESP_LOGE(TAG, "Not accepting request: previous request is still being serviced");
            ESP_LOGE(TAG, "Are you retrying this request immediatetely after a timeout?");
            ESP_LOGE(TAG, "In this case, you may need to increase your client-side timeout");
            ESP_LOGE(TAG, "Errors beyond this line may no longer be relevant");
            return;
        }

        userport->receivePartial(header, protocol->requestHeaderSize(), onRequestHeaderReceived, onRequestHeaderAborted);
    }

    void Service::onRequestHeaderAborted(uint8_t *header, uint32_t bytes_received) {
        ESP_LOGE(TAG, "Failed to receive request header");
        ESP_LOGW(TAG, "Received %d of %d bytes",
            bytes_received, service->protocol->requestHeaderSize());
    }

    void Service::onRequestHeaderReceived(uint8_t *header, uint32_t size) {
        ESP_LOGD(TAG, "Parsing %s request header...", service->protocol->name());

        service->request = service->protocol->createRequest(header);
        service->command = Command::create(service->request);

        if (customTransferTimeout) {
            transferTimeout = customTransferTimeout;
            customTransferTimeout = 0;
            ESP_LOGV(TAG, "Using previously requested transfer timeout of %dms for this request", transferTimeout);
        }
        else {
            transferTimeout = WIC64_DEFAULT_TRANSFER_TIMEOUT;
            ESP_LOGV(TAG, "Using default transfer timeout of %dms", transferTimeout);
        }

        if (customRemoteTimeout) {
            remoteTimeout = customRemoteTimeout;
            customRemoteTimeout = 0;
            ESP_LOGV(TAG, "Using previously requested HTTP request timeout of %dms for this request", remoteTimeout);
        }
        else {
            remoteTimeout = WIC64_DEFAULT_REMOTE_TIMEOUT;
            ESP_LOGV(TAG, "Using default HTTP request timeout of %dms", remoteTimeout);
        }

        if (!service->command->supportsProtocol()) {
            ESP_LOGE(TAG, "Command 0x%02x (%s) not supported by %s protocol ('%c')",
                service->command->id(),
                service->command->describe(),
                service->protocol->name(),
                service->protocol->id());

            service->finalizeRequest("Command not supported by protocol", false);
            return;
        }

        if (!service->request->hasPayload()) {
            service->onRequestReceived();
            return;
        }

        ESP_LOGI(TAG, "Receiving request payload");
        Data* payload = service->request->payload();

        if (payload->size() < 0x10000) {
            service->receiveStaticRequest();
        }
        else {
            if(!service->command->supportsQueuedRequest()) {
                ESP_LOGE(TAG, "Command 0x%02x (%s) does not support sending payloads >=64kb",
                    service->command->id(),
                    service->command->describe());

                service->finalizeRequest("Command does not support sending payloads >=64kb", false);
                return;
            }
            ESP_LOGI(TAG, "Starting queued receive of %d bytes", payload->size());

            payload->queue(transferQueue, payload->size());
            service->receiveQueuedRequest();
            service->onRequestReceived();
        }
    }

    void Service::queueTask(void *payload_size_ptr) {
        service->receiveQueuedRequest();
        vTaskDelete(NULL);
    }

    void Service::receiveQueuedRequest() {
        Data *payload = command->request()->payload();

        ESP_LOGD(TAG, "Preparing queued receive of %d bytes", payload->size());

        bytes_remaining = payload->size();
        items_remaining = WIC64_QUEUE_ITEMS_REQUIRED(payload->size());

        service->receiveQueuedRequestData();
    }

    void Service::receiveQueuedRequestData(uint8_t *data, uint32_t bytes_received) {
        ESP_LOGV(TAG, "%s call of receiveQueuedRequestData(), %d bytes in %d item%s remaining",
            (data == NULL) ? "First" : "Subsequent",
            service->bytes_remaining,
            service->items_remaining,
            (service->items_remaining > 1) ? "s" : "");

        if (data != NULL) {
            if (xQueueSend(transferQueue, data, pdMS_TO_TICKS(transferTimeout)) != pdTRUE) {
                ESP_LOGW(TAG, "Could not send next item to receive queue in %dms", transferTimeout);
                onRequestAborted(NULL, service->bytes_remaining);
            };
            service->bytes_remaining -= bytes_received;
            service->items_remaining--;
        }

        uint32_t size = (service->items_remaining > 1)
            ? WIC64_QUEUE_ITEM_SIZE
            : service->bytes_remaining;

        (service->items_remaining > 1)
            ? userport->receivePartial(transferQueueSendBuffer, size, receiveQueuedRequestData, onRequestAborted)
            : userport->receive(transferQueueSendBuffer, size, receiveQueuedRequestData, onRequestAborted);
    }

    void Service::receiveStaticRequest(void) {
        Data* payload = service->request->payload();
        userport->receive(payload->data(), payload->size(), onRequestReceived, onRequestAborted);
    }

    void Service::onRequestAborted(uint8_t *data, uint32_t bytes_received) {
        Data* payload = service->request->payload();

        ESP_LOGW(TAG, "Received %d of %d bytes",
            bytes_received, service->request->payload()->size());

        if (payload->isQueued()) {
            service->command->abort();
        }
        service->finalizeRequest("Transfer aborted while receiving request", false);
    }

    void Service::onRequestReceived(uint8_t *ignoredData, uint32_t ignoredSize) {
        Request* request = service->request;
        Data* payload = request->payload();

        ESP_LOGI(TAG, "Request received successfully");
        ESP_LOGI(TAG, "Handling request [" WIC64_FORMAT_CMD WIC64_GREEN("]"), request->id());

        if (request->hasPayload() && !request->payload()->isQueued()) {
            ESP_LOG_HEXV(TAG, "Payload", payload->data(), payload->size());
        }

        ESP_LOGI(TAG, WIC64_SEPARATOR);
        ESP_LOGI(TAG, "Executing command " WIC64_WHITE("") "%s", service->command->describe());
        ESP_LOGI(TAG, WIC64_SEPARATOR);
        Command::execute(service->command);
    }

    void Service::onResponseReady(void *arg, esp_event_base_t base, int32_t id, void *data) {
        service->sendResponse();
    }

    void Service::sendResponse() {
        sendResponseHeader();
    }

    void Service::sendResponseHeader() {
        static uint8_t header[Protocol::MAX_REQUEST_HEADER_SIZE];
        response = command->response();

        protocol->setResponseHeader(header, command->status(), response->sizeToReport());

        response->isPresent()
            ? userport->sendPartial(header, protocol->responseHeaderSize(), onResponseHeaderSent, onResponseHeaderAborted)
            : userport->send(header, protocol->responseHeaderSize(), onResponseHeaderSent, onResponseHeaderAborted);
    }

    void Service::onResponseHeaderAborted(uint8_t *data, uint32_t bytes_sent) {
        ESP_LOGW(TAG, "Sent %d of %d bytes", bytes_sent, service->protocol->responseHeaderSize());
        service->finalizeRequest("Aborted while sending response header", false);
    }

    void Service::onResponseHeaderSent(uint8_t* data, uint32_t size) {
        Data *response = service->response;

        response->isEmpty()
            ? service->finalizeRequest("Request handled successfully", true)
            : response->isQueued()
                ? service->sendQueuedResponse()
                : service->sendStaticResponse();
    }

    void Service::sendQueuedResponse() {
        Data *response = command->response();

        if (!service->command->supportsQueuedResponse()) {
            ESP_LOGE(TAG, "Command 0x%02x (%s) does not support receiving "
                          "payloads >=64kb when using %s protocol ('%c')",
                service->command->id(),
                service->command->describe(),
                service->protocol->name(),
                service->protocol->id());

            service->finalizeRequest("Command does not support receiving payloads >=64kb", false);
            return;
        }

        ESP_LOGD(TAG, "Preparing queued send of %d bytes", response->size());

        bytes_remaining = response->size();
        items_remaining = WIC64_QUEUE_ITEMS_REQUIRED(response->size());

        service->sendQueuedResponseData();
    }

    void Service::sendQueuedResponseData(uint8_t *isSubsequentCall, uint32_t ignoreSize) {
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

        uint32_t size = (service->items_remaining > 1)
            ? WIC64_QUEUE_ITEM_SIZE
            : service->bytes_remaining;

        if (xQueueReceive(response->queue(), transferQueueReceiveBuffer, pdMS_TO_TICKS(transferTimeout)) != pdTRUE) {
            ESP_LOGW(TAG, "Could not read next item from response queue in %dms", transferTimeout);
            onResponseAborted(NULL, service->bytes_remaining);
            return;
        }
        service->bytes_remaining -= size;

        (service->items_remaining > 1)
            ? userport->sendPartial((uint8_t*) transferQueueReceiveBuffer, size, sendQueuedResponseData, onResponseAborted)
            : userport->send((uint8_t*) transferQueueReceiveBuffer, size, onResponseSent, onResponseAborted);
    }

    void Service::sendStaticResponse(void) {
        userport->send(response->data(), response->size(), onResponseSent, onResponseAborted);
    }

    void Service::onResponseAborted(uint8_t *data, uint32_t bytes_sent) {
        Data *response = service->command->response();
        ESP_LOGW(TAG, "Sent %d of %d bytes", bytes_sent, response->size());

        service->finalizeRequest("Aborted while sending response", false);
    }

    void Service::onResponseSent(uint8_t *data, uint32_t size) {
        if (!service->response->isQueued()) {
            ESP_LOG_HEXV(TAG, "Response", data, size);
        }
        service->finalizeRequest("Request handled successfully", true);
    }

    void Service::finalizeRequest(const char* message, bool success) {
        esp_log_level_t level;

        level = success ? ESP_LOG_INFO : ESP_LOG_WARN;
        ESP_LOG_LEVEL(level, TAG, "Finalizing request: %s", message);

        if (command != NULL) {
            if (request->payload()->isQueued()) {
                ESP_LOG_LEVEL(ESP_LOG_DEBUG, TAG, "Resetting request queue");
                xQueueReset(request->payload()->queue());
            }

            if (command->response()->isQueued()) {
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
                "firmware bug or protocol violation");
        }

        vTaskDelay(pdMS_TO_TICKS(5));
        log_free_mem(TAG, ESP_LOG_VERBOSE);
    }
}
