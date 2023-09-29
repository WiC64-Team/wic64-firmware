#ifndef WIC64_HTTP_CLIENT_H
#define WIC64_HTTP_CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "WString.h"

#include "data.h"
#include "command.h"

namespace WiC64 {
    class HttpClient {
        public: static const char* TAG;

        private:
            esp_http_client_handle_t m_client = NULL;
            QueueHandle_t m_queue = NULL;

            static const uint16_t MAX_URL_LENGTH = 2000; // see https://stackoverflow.com/a/417184
            static const uint8_t MAX_RETRIES = 10;
            int32_t m_statusCode = -1;

            esp_http_client_handle_t handle() { return m_client; }
            QueueHandle_t queue() { return m_queue; }

            const char* HEADER = "--WiC64-Binary-Data\nContent-Disposition: form-data; name=\"data\"\n\n";
            const char* FOOTER = "\n--WiC64-Binary-Data--\n";

            void closeConnection(void);
            bool isConnectionClosed();

            static esp_err_t eventHandler(esp_http_client_event_t *evt);
            static void queueTask(void* content_length_ptr);

            void request(Command *command, esp_http_client_method_t method, String& url, Data* data);

        public:
            HttpClient();
            int32_t statusCode() { return m_statusCode; }

            void get(Command* command, String& url);
            void post(Command* command, String& url, Data* data);
    };
}

#endif // WIC64_HTTP_CLIENT_H