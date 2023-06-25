#include "esp32-hal.h"
#include "esp_event.h"

#include "wic64.h"
#include "service.h"
#include "userport.h"
#include "data.h"
#include "command.h"
#include "utilities.h"
namespace WiC64 {
    const char* Service::TAG = "SERVICE";

    extern Service *service;
    extern Userport *userport;

    bool Service::supports(uint8_t api) {
        return api == WiC64::API_V1;
    }

    void Service::receiveRequest(uint8_t api) {
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

        if (!Command::defined(id)) {
            ESP_LOGD(TAG, "Undefined command id requested: 0x%02x, aborting", id);
            return;
        }

        uint16_t argument_size = (*((uint16_t*) header)) - API_V1_ARGUMENT_SIZE_CORRECTION;
        bool has_argument = (argument_size > 0);

        service->request = new Request(api, id, has_argument ? 1 : 0);

        ESP_LOGD(TAG, "request=0x%02X argc=%d size=%d",
            service->request->id(),
            service->request->argc(),
            argument_size);

        if (service->request->hasArguments()) {
            ESP_LOGI(TAG, "Receiving request argument");
            Data* argument = service->request->addArgument(new Data(argument_size));
            userport->receive(argument->data(), argument->size(), onRequestReceived, onRequestAborted);
        }
        else {
            service->onRequestReceived();
        }
    }

    void Service::onRequestAborted(uint8_t *data, uint16_t bytes_received) {
        ESP_LOGW(TAG, "Received only %d bytes of %d bytes expected",
            bytes_received, service->request->argument(0)->size());
        ESP_LOGW(TAG, "Aborted while receiving request => Freeing allocated memory");

        delete service->request;
        service->request = NULL;
    }

    void Service::onRequestReceived(void) {
        onRequestReceived(NULL, 0);
    }

    void Service::onRequestReceived(uint8_t *ignoredData, uint16_t ignoredSize) {
        Request *request = service->request;

        ESP_LOGI(TAG, "Request received successfully");
        ESP_LOGI(TAG, "Handling request 0x%02X:", request->id());
        ESP_LOGV(TAG, "Request arguments...");

        for (uint8_t i=0; i<request->argc(); i++) {
            Data* argument = request->argument(i);
            static char title[12+1];
            snprintf(title, 12, "argument %hhu", i);
            ESP_LOG_HEXV(TAG, title, argument->data(), argument->size());
        }

        service->command = Command::create(request);
        service->response = service->command->execute();

        service->sendResponse();
    }

    void Service::sendResponse() {
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
            ? userport->sendPartial(responseSizeBuffer, 2, onResponseSizeSent, onResponseSizeAborted)
            : userport->send(responseSizeBuffer, 2, onResponseSizeSent, onResponseSizeAborted);
    }

    void Service::onResponseSizeAborted(uint8_t *data, uint16_t bytes_sent) {
        ESP_LOGW(TAG, "Sent only %d of 2 bytes", bytes_sent);
        service->finalizeRequest("Aborted while sending response size", false);
    }

    void Service::onResponseSizeSent(uint8_t* data, uint16_t size) {
        Data *response = service->response;

        response->isPresent()
            ? userport->send(response->data(), response->size(), onResponseSent, onResponseAborted)
            : service->finalizeRequest("Request handled successfully", true);
    }

    void Service::onResponseAborted(uint8_t *data, uint16_t bytes_sent) {
        ESP_LOGW(TAG, "Sent only %d of %d bytes", bytes_sent, service->response->size());
        service->finalizeRequest("Aborted while sending response", false);
    }

    void Service::onResponseSent(uint8_t *data, uint16_t size) {
        ESP_LOG_HEXV(TAG, "response", data, size);
        service->finalizeRequest("Request handled successfully", true);
    }

    void Service::finalizeRequest(const char* message, bool success) {
        esp_log_level_t level = success ? ESP_LOG_INFO : ESP_LOG_WARN;
        ESP_LOG_LEVEL(level, TAG, "%s => freeing allocated memory", message);

        if (command != NULL) {
            delete command;
            command = NULL;
        }

        log_free_mem();
    }
}
