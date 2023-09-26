#ifndef WIC64_COMMAND_H
#define WIC64_COMMAND_H

#include <functional>

#include "wic64.h"
#include "service.h"

namespace WiC64 {
    class Command {
        private:
            Request* m_request = NULL;
            Data* m_response = NULL;
            bool m_response_ready = false;

        public:
            const static char* TAG;

            static Command* create(Request* request);

            Command(Request* request);
            virtual ~Command();

            uint8_t id(void);

            bool isLegacyRequest() { return m_request->api() == WiC64::API_LAYER_1; }

            Request* request(void) { return m_request; }
            Data* response(void) { return m_response; }
            bool isResponseReady() { return m_response_ready; }

            virtual const char* describe();
            virtual void execute(void);
            virtual void responseReady();
    };
}
#endif // WIC64_COMMAND_H
