#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include <functional>

#include "wic64.h"
#include "service.h"
#include "protocol.h"

namespace WiC64 {
    class Command {
        private:
            uint8_t m_status_code = SUCCESS;
            static char m_status_message[256];

            Request* m_request = NULL;
            Data* m_response = NULL;
            bool m_response_ready = false;
            bool m_aborted = false;

        public:
            const static char* TAG;

            const static uint8_t SUCCESS          = 0;
            const static uint8_t INTERNAL_ERROR   = 1;
            const static uint8_t CLIENT_ERROR     = 2;
            const static uint8_t CONNECTION_ERROR = 3;
            const static uint8_t NETWORK_ERROR    = 4;
            const static uint8_t SERVER_ERROR     = 5;

            static Command* create(Request* request);
            static void execute(Command* command);
            static void commandTask(void* command);
            static const char* statusMessage(void) { return m_status_message; }

            Command(Request* request);
            virtual ~Command();

            uint8_t id(void) { return request()->id(); }

            bool isLegacyRequest() { return m_request->protocol()->id() == Protocol::LEGACY; }

            Request* request(void) { return m_request; }
            Data* response(void) { return m_response; }
            bool isResponseReady() { return m_response_ready; }

            void abort(void) { m_aborted = true; }
            bool aborted(void) { return m_aborted; }

            void success(const char* message) { status(SUCCESS, message); }
            void success(const char* message, const char* legacy_message) { status(SUCCESS, message, legacy_message); }

            void error(uint8_t code, const char* message) { status(code, message, message); }
            void error(uint8_t code, const char* message, const char* legacy_message) { status(code, message, legacy_message); }

            uint8_t status(void) { return m_status_code; };
            void status(uint8_t code, const char* message) { status(code, message, message); }
            void status(uint8_t code, const char* message, const char* legacy_message);

            virtual bool supportsProtocol();
            virtual bool supportsQueuedRequest();
            virtual bool supportsQueuedResponse();

            virtual const char* describe();
            virtual void execute(void);
            virtual void responseReady();
    };
}
#endif // WIC64_COMMAND_H
