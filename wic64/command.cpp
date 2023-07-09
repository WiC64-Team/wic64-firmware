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
        uint8_t num_commands = sizeof(commands) / sizeof(command_map_entry_t);
        command_map_entry_t command;

        for (uint8_t i=0; i<num_commands; i++) {
            command = commands[i];
            if (command.id == id) {
                return true;
            }
        }
        return false;
    }

    Command* Command::create(Request* request) {
        uint8_t num_commands = sizeof(commands) / sizeof(command_map_entry_t);
        command_map_entry_t command;

        for (uint8_t i=0; i<num_commands; i++) {
            command = commands[i];
            if (command.id == request->id()) {
                return command.create(request);
            }
        }
        return nullptr;
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
