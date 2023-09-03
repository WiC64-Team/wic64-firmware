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

    uint8_t Command::id() {
        return request()->id();
    }

    Command* Command::create(Request* request) {
        command_map_entry_t command;

        for (uint8_t i=0; commands[i].id != WIC64_CMD_NONE; i++) {
            command = commands[i];
            if (command.id == request->id()) {
                return command.create(request);
            }
        }
        return WIC64_COMMAND_UNDEFINED.create(request);
    }

    const char *Command::describe() {
        return "Generic command (no description available)";
    }

    void Command::execute(void)
    {
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
