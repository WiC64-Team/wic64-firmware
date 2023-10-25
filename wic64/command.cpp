#include "command.h"
#include "commands/commands.h"
#include "service.h"

#include "esp_event.h"

namespace WiC64 {
    const char* Command::TAG = "COMMAND";

    extern Service *service;

    char Command::m_status_message[256] = "";

    Command::Command(Request* request) {
        m_request = request;
        m_response = new Data();
        m_status_code = SUCCESS;
    }

    Command::~Command() {
        delete m_request;
        delete m_response;
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

    void Command::execute(Command *command) {
        if (command->request()->payload()->isQueued()) {
            xTaskCreatePinnedToCore(commandTask, "COMMAND", 4096, command, 20, NULL, 1);
        } else {
            command->execute();
        }
    }

    void Command::commandTask(void *command) {
        ((Command*) command)->execute();
        vTaskDelete(NULL);
    }

    void Command::status(uint8_t code, const char *message, const char* legacy_message) {
        if (isLegacyRequest()) {
            ESP_LOGD(TAG, "Sending legacy %s message \"%s\" (reason: %s)",
                code ? "error" : "success",
                legacy_message,
                message);
            response()->copyData(legacy_message);
        } else {
            ESP_LOGD(TAG, "Setting status: code = %d, message = \"%s\"", code, message);
            m_status_code = code;
            strncpy(m_status_message, message, 255);
        }
    }

    bool Command::supportsProtocol() {
        switch (m_request->protocol()->id()) {
            case Protocol::LEGACY:   return true;  break;
            case Protocol::STANDARD: return true;  break;
            case Protocol::EXTENDED: return false; break;
            default: return false;  break;
        }
    }

    bool Command::supportsQueuedRequest() {
        return false;
    }

    bool Command::supportsQueuedResponse() {
        return m_request->protocol()->id() == Protocol::EXTENDED;
    }

    const char *Command::describe() {
        return "Generic command (no description available)";
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
