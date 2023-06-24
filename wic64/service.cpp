#include "esp32-hal.h"
#include "esp_event.h"

#include "service.h"
#include "userport.h"
#include "data.h"
#include "command.h"
#include "utilities.h"
namespace WiC64 {
    extern Service *service;
    extern Userport *userport;

    bool Service::supports(uint8_t api) {
        return api == API_V1_ID;
    }

    void Service::receiveRequest(uint8_t api) {
        static uint8_t header[REQUEST_HEADER_SIZE];

        if (api == API_V1_ID) {
            userport->receivePartial(header, REQUEST_HEADER_SIZE, parseRequestHeaderV1);
        }
    }

    void Service::parseRequestHeaderV1(uint8_t *header, uint16_t size) {
        log_d("Parsing v1 request header...");
        log_data("Header", (uint8_t*) header, size);

        uint8_t api = API_V1_ID;
        uint8_t id = header[2];

        if (!Command::defined(id)) {
            log_d("Undefined command id requested: 0x%02x, aborting", id);
            return;
        }

        uint16_t argument_size = (*((uint16_t*) header)) - API_V1_REQUEST_SIZE;
        bool has_argument = (argument_size > 0);

        service->request = new Request(api, id, has_argument ? 1 : 0);

        log_d("request=0x%02X argc=%d size=%d",
            service->request->id(),
            service->request->argc(),
            argument_size);

        if (service->request->hasArguments()) {
            Data* argument = service->request->addArgument(new Data(argument_size));
            userport->receive(argument->data(), argument->size(), onRequestReceived, onRequestAborted);
        }
        else {
            service->onRequestReceived();
        }
    }

    void Service::onRequestAborted(uint8_t *data, uint16_t bytes_received) {
        log_e("Received %d bytes, %d expected",
            bytes_received, service->request->argument(0)->size());
        log_d("Freeing memory allocated for request");

        delete service->request;
        service->request = NULL;
    }

    void Service::onRequestReceived(void) {
        onRequestReceived(NULL, 0);
    }

    void Service::onRequestReceived(uint8_t *ignoredData, uint16_t ignoredSize) {
        Request *request = service->request;

        log_d("Handling request 0x%02X:", request->id());
        log_v("Request arguments...");

        for (uint8_t i=0; i<request->argc(); i++) {
            Data* argument = request->argument(i);
            static char title[12+1];
            snprintf(title, 12, "Argument %hhu", i);
            log_data(title, argument->data(), argument->size());
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

        response->isPresent()
            ? userport->sendPartial(responseSizeBuffer, 2, onResponseSizeSent)
            : userport->send(responseSizeBuffer, 2, onResponseSizeSent);
    }

    void Service::onResponseSizeSent(uint8_t* data, uint16_t size) {
        Data *response = service->response;
        log_data("Response size", data, size);

        response->isPresent()
            ? userport->send(response->data(), response->size(), onResponseSent, onResponseAborted)
            : service->finalizeRequest("Request handled successfully");
    }

    void Service::onResponseAborted(uint8_t *data, uint16_t bytes_sent) {
        log_e("Sent %d of %d bytes", bytes_sent, service->response->size());
        service->finalizeRequest("Aborted while sending response");
    }

    void Service::onResponseSent(uint8_t *data, uint16_t size) {
        log_data("Response", data, size);
        service->finalizeRequest("Request handled successfully");
    }

    void Service::finalizeRequest(const char* result) {
        log_d("%s => freeing allocated memory", result);

        if (command != NULL) {
            delete command;
            command = NULL;
        }

        log_free_mem();
    }
}
