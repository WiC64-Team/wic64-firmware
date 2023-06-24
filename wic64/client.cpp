#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "WString.h"

#include "client.h"
#include "utilities.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

namespace WiC64 {
    extern Client* client;

    const char* Client::TAG = "CLIENT";

    esp_err_t Client::event_handler(esp_http_client_event_t *evt) {
        int bytes_received = 0;

        switch(evt->event_id) {
            case HTTP_EVENT_ERROR:
                ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
                break;

            case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
                break;

            case HTTP_EVENT_HEADER_SENT:
                ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
                break;

            case HTTP_EVENT_ON_HEADER:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER: %s: %s", evt->header_key, evt->header_value);
                break;

            case HTTP_EVENT_ON_DATA:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);

                bytes_received = MIN(evt->data_len, (0x10000 - client->size()));

                if (bytes_received) {
                    memcpy(client->buffer() + client->size(), evt->data, bytes_received);
                }
                client->size(client->size() + bytes_received);
                break;

            case HTTP_EVENT_ON_FINISH:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
                break;

            case HTTP_EVENT_DISCONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
                break;
        }
        return ESP_OK;
    }

    Client::Client() {
        m_buffer = (uint8_t*) calloc(0x10000, sizeof(uint8_t));

        if (m_buffer == NULL) {
            ESP_LOGE(TAG, "Failed to allocate response buffer");
        }
    }

    Data *Client::get(String url) {
        int status;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        esp_http_client_config_t config = {
            .url = url.c_str(),
            .event_handler = event_handler,
        };

        #pragma GCC diagnostic pop

        esp_http_client_handle_t client = esp_http_client_init(&config);

        if (client == NULL) {
            return new Data();
        }
        size(0);

        esp_err_t result = esp_http_client_perform(client);

        if (result == ESP_OK) {
            status = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status, m_size);
        } else {
            ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(result));
        }

        esp_http_client_cleanup(client);

        return new Data(m_buffer, (uint16_t) m_size);
    }
}
