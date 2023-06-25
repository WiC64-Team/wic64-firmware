#include "wic64.h"
#include "client.h"
#include "connection.h"
#include "utilities.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "WString.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

namespace WiC64 {
    const char* Client::TAG = "CLIENT";

    extern Client* client;
    extern Connection *connection;

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

    retry:
        if (m_handle == NULL) {
            esp_http_client_config_t config = {
                .url = url.c_str(),
                .event_handler = event_handler,
            };
            m_handle = esp_http_client_init(&config);
        } else {
            esp_http_client_set_url(m_handle, url.c_str());
        }

        #pragma GCC diagnostic pop

        if (client == NULL) {
            return new Data();
        }
        size(0);

        esp_err_t result = esp_http_client_perform(m_handle);

        if (result == ESP_OK) {
            status = esp_http_client_get_status_code(m_handle);
            ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status, m_size);
        }
        else {
            ESP_LOGE(TAG, "HTTP GET: %s", esp_err_to_name(result));
            if (result == ESP_ERR_HTTP_FETCH_HEADER) {
                ESP_LOGW(TAG, "Assuming keep-alive connection closed by peer, opening new connection");
                cleanup();

                if (connection->isConnected()) {
                    goto retry; // TODO: check all error codes, add proper break condition(s)
                }
            }
        }
        return new Data(m_buffer, (uint16_t) m_size);
    }

    void Client::cleanup(void) {
        esp_http_client_cleanup(m_handle);
        m_handle = NULL;
        m_size = 0;
    }
}
