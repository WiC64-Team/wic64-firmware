#include "wic64.h"
#include "client.h"
#include "connection.h"
#include "utilities.h"
#include "commands/httpGet.h"


#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "WString.h"

namespace WiC64 {
    const char* Client::TAG = "CLIENT";

    extern Client* client;
    extern Connection *connection;

    Client::Client() {
        m_queue = xQueueCreate(WIC64_QUEUE_SIZE, WIC64_QUEUE_ITEM_SIZE);

        if (m_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create queue");
        }
    }

    esp_err_t Client::eventHandler(esp_http_client_event_t *evt) {
        static bool connection_header_sent = false;

        switch(evt->event_id) {
            case HTTP_EVENT_ERROR:
                if(evt->data != NULL && evt->data_len > 0) {
                    ESP_LOG_HEXD(TAG, "Error event data", (uint8_t*) evt->data, evt->data_len);
                }
                ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
                break;

            case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
                break;

            case HTTP_EVENT_HEADER_SENT:
                ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
                connection_header_sent = false;
                break;

            case HTTP_EVENT_ON_HEADER:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER: %s: %s", evt->header_key, evt->header_value);

                if(strcmp(evt->header_key, "Connection") == 0) {
                    connection_header_sent = true;
                    client->keepAlive(strcasecmp(evt->header_value, "keep-alive") == 0);
                }
                break;

            case HTTP_EVENT_ON_DATA:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, size=%d", evt->data_len);
                break;

            case HTTP_EVENT_ON_FINISH:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
                if (!connection_header_sent) {
                    client->keepAlive(false);
                }

                break;

            case HTTP_EVENT_DISCONNECTED:
                ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
                break;
        }
        return ESP_OK;
    }

    void Client::get(Command *command, String url) {
        static uint8_t data[0x10000];
        int32_t size = 0;

        int32_t result;
        int32_t status;
        int32_t content_length;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        esp_http_client_config_t config = {
            .url = url.c_str(),
            .timeout_ms = 5 * 1000,
            .event_handler = eventHandler,
        };

        #pragma GCC diagnostic pop

        closeUnlessKeptAlive();

    RETRY:
        if (m_client == NULL) {
            ESP_LOGW(TAG, "Opening new connection");
            m_client = esp_http_client_init(&config);

            if (m_client == NULL) {
                ESP_LOGE(TAG, "Could not create HTTP client");
                command->responseReady();
                return;
            }
        } else {
            if(esp_http_client_set_url(m_client, url.c_str()) == ESP_FAIL) {
                command->responseReady();
                return;
            };
        }

        if ((result = esp_http_client_open(m_client, 0) != ESP_OK)) {
            ESP_LOGE(TAG, "Failed to open connection: %s", esp_err_to_name(result));
            close();
            goto RETRY;
        }

        if ((result = esp_http_client_fetch_headers(m_client)) == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to fetch headers");
            close();
            goto RETRY;
        }
        content_length = result;
        status = (HttpStatus_Code) esp_http_client_get_status_code(m_client);

        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status, content_length);
        ESP_LOGW(TAG, "Keep alive: %s", m_keepAlive ? "true" : "false");

        if (content_length > 0) {  // Content-Length specified by peer -> start a queued transfer
            ESP_LOGI(TAG, "Starting queued send of %d bytes", content_length);

            // Set the queue and the size of the reponse in the response object
            command->response()->queue(client->queue(), (uint16_t) content_length);

            // Ask the command to send the SERVICE_RESPONSE_READY event so that
            // Service will start reading and sending data down to the C64 as soon as
            // it becomes available
            command->responseReady();

            // Start the queueing task that reads from the connection and inserts it at the end
            // of the queue in chunks of WIC64_QUEUE_ITEM_SIZE. Well pass a pointer to the
            // content_length variable via the pvParameters argument.
            xTaskCreatePinnedToCore(queueTask, "SENDER", 4096, &content_length, 10, NULL, 1);
        }
        else { // Content-Length not send by server or Transfer-Encoding: chunked
            ESP_LOGI(TAG, "Reading response data of unknown length (limited to 64kb)");

            // Read up to 64kb from the connection into the static receive buffer
            if ((size = esp_http_client_read(m_client, (char*) data, 0x10000)) == -1) {
                ESP_LOGE(TAG, "Read Error");
            }
            ESP_LOGI(TAG, "Read %d bytes", size);
        }

        // If the response was not queued, put data received in buffer into response
        if(!command->response()->isQueued()) {
            size = size < 0 ? 0 : size;
            command->response()->data(data, size);
            command->responseReady();
        }
    }

    void Client::close(void) {
        if (m_client != NULL) {
            ESP_LOGW(TAG, "Closing previously used connection");
            esp_http_client_close(m_client);
            esp_http_client_cleanup(m_client);
            m_client = NULL;
        }
    }

    void Client::closeUnlessKeptAlive() {
        if (!m_keepAlive) {
            close();
        }
    }

    void Client::queueTask(void *content_length_ptr) {
        int32_t content_length = *((int32_t *) content_length_ptr);
        static uint8_t data[WIC64_QUEUE_ITEM_SIZE];

        int32_t bytes_read = 0;
        int32_t total_bytes_read = 0;

        ESP_LOGD(TAG, "Client queue task queueing %d bytes...", content_length);

        do {
            bytes_read = esp_http_client_read(client->handle(), (char*) data, WIC64_QUEUE_ITEM_SIZE);

            if (bytes_read == -1) {
                ESP_LOGE(TAG, "Read Error");
                break;
            }
            xQueueSend(client->queue(), data, pdMS_TO_TICKS(1000));

            total_bytes_read += bytes_read;

        } while (total_bytes_read < content_length);

        vTaskDelete(NULL);
    }
}
