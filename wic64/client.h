#ifndef WIC64_CLIENT_H
#define WIC64_CLIENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"

#include "data.h"

namespace WiC64 {
    class Client {
        private:
            uint8_t* m_buffer;
            int m_size = 0;
            static esp_err_t event_handler(esp_http_client_event_t *evt);

        public:
            int size() { return m_size; }
            void size(int size ) { m_size = size; }
            uint8_t* buffer() { return m_buffer; }

            Client();
            Data* get(const char* url);
    };
}

#endif // WIC64_CLIENT_H