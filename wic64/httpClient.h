#ifndef WIC64_HTTP_CLIENT_H
#define WIC64_HTTP_CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "WString.h"

#include "data.h"
#include "command.h"
#include "utilities.h"

namespace WiC64 {
    class HttpClient {
        public: static const char* TAG;

        private:
            esp_http_client_handle_t m_client = NULL;

            static const uint16_t MAX_URL_LENGTH = 0x2000;
            static const uint8_t MAX_RETRIES = 3;

            int32_t m_statusCode = -1;
            char m_postUrl[MAX_URL_LENGTH+1] = { '\0' };

            uint8_t retries;
            int32_t timeRequestStarted;

            bool canRetry(Command* command);
            const char* statusToString(int32_t code);
            esp_http_client_handle_t handle() { return m_client; }

            const char* HEADER = "--WiC64-Binary-Data\nContent-Disposition: form-data; name=\"data\"" CRLF CRLF;
            const char* FOOTER = CRLF "--WiC64-Binary-Data--" CRLF;

            void closeConnection(void);
            bool isConnectionClosed();

            static esp_err_t eventHandler(esp_http_client_event_t *evt);
            static void queueTask(void* content_length_ptr);

            void request(Command *command, esp_http_client_method_t method, const char* url, Data* data);

        public:
            HttpClient();
            int32_t statusCode() { return m_statusCode; }

            void get(Command* command, String& url);
            const char* postUrl(void) { return m_postUrl; }
            void postUrl(String& url);
            void postData(Command* command, Data* data);
    };
}

#endif // WIC64_HTTP_CLIENT_H