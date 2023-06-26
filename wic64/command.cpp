#include "command.h"
#include "commands/commands.h"
#include "service.h"

#include "esp_event.h"

namespace WiC64 {
    const char* Command::TAG = "COMMAND";

    extern Service *service;

    Command::Command(Request* request) {
        m_request = request;
        m_response = new Data();
    }

    Command::~Command() {
        delete m_request;
        delete m_response;
    }

    bool Command::defined(uint8_t id) {
        return commands.find(id) != commands.end();
    }

    Command* Command::create(Request* request) {
        if (defined(request->id())) {
            return commands.at(request->id())(request);
        }
        return nullptr;
    }

    void Command::execute(void) {
        responseReady();
    }

    void Command::responseReady() {
        ESP_LOGD(TAG, "Posting SERVICE_RESPONSE_READY event");
        esp_event_post_to(
            service->eventLoop(),
            SERVICE_EVENTS,
            SERVICE_RESPONSE_READY,
            NULL, 0, 0);

        m_response_ready = true;
    }
}
