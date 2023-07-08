#include "wic64.h"
#include "client.h"
#include "connection.h"
#include "utilities.h"
#include "settings.h"
#include "commands/httpGet.h"
#include "version.h"

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
    extern Settings *settings;

    Client::Client() {
        m_queue = xQueueCreate(WIC64_QUEUE_SIZE, WIC64_QUEUE_ITEM_SIZE);

        if (m_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create queue");
        }
    }

    esp_err_t Client::eventHandler(esp_http_client_event_t *event) {
        static bool connection_header_sent = false;

        switch(event->event_id) {
            case HTTP_EVENT_ERROR:
                if(event->data != NULL && event->data_len > 0) {
                    ESP_LOG_HEXD(TAG, "Error event data", (uint8_t*) event->data, event->data_len);
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
                ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER: %s: %s", event->header_key, event->header_value);

                // Connection: keep-alive
                if (strcmp(event->header_key, "Connection") == 0) {
                    connection_header_sent = true;
                    client->keepAlive(strcasecmp(event->header_value, "keep-alive") == 0);
                }

                // WiC64-Security-Token-Key: <key>
                if (strcmp(event->header_key, "WiC64-Security-Token-Key") == 0) {
                    settings->securityTokenKey(event->header_value);
                }
                break;

            case HTTP_EVENT_ON_DATA:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, size=%d", event->data_len);
                if (!connection_header_sent) {
                    client->keepAlive(false);
                }
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

    void Client::get(Command *command, String url) {
        int32_t size = 0;

        int32_t status_code = -1;
        static int32_t content_length;

        int32_t result;

        // REDESIGN: This is the legacy error response. We need to send
        // qualified error information in the future.
        static char error[] = "!0";

        uint8_t retries = 3;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        esp_http_client_config_t config = {
            .url = url.c_str(),
            .user_agent = "WiC64/" WIC64_VERSION_STRING,
            .method = HTTP_METHOD_GET,
            .timeout_ms = 5 * 1000,
            .event_handler = eventHandler,
        };

        #pragma GCC diagnostic pop

        closeConnectionUnlessKeptAlive();

    RETRY:
        if (isConnectionClosed()) {
            ESP_LOGV(TAG, "Opening new connection");
            m_client = esp_http_client_init(&config);

            if (m_client == NULL) {
                ESP_LOGE(TAG, "Failed to create esp_http_client");
                goto ERROR;
            }
        } else {
            if(esp_http_client_set_url(m_client, url.c_str()) == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to set URL");
                goto ERROR;
            };
        }

        if ((result = esp_http_client_open(m_client, 0) != ESP_OK)) {
            ESP_LOGE(TAG, "Failed to open connection: %s", esp_err_to_name(result));

            const char* reason = m_keepAlive
                ? "Assuming keep-alive connection timed out"
                : "Connection failed for unknown reasons";

            ESP_LOGW(TAG, "%s, retrying %d more time%s...",
                reason, retries, (retries > 1) ? "s" : "");

            if (retries-- > 0) {
                closeConnection();
                goto RETRY;
            } else {
                goto ERROR;
            }
        }

        if ((result = esp_http_client_fetch_headers(m_client)) == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to fetch headers");
            goto ERROR;
        }

        content_length = result;
        status_code = esp_http_client_get_status_code(m_client);

        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d", status_code, content_length);
        ESP_LOGI(TAG, "Keep alive: %s", m_keepAlive ? "true" : "false");

        if (content_length > 0x10000) {
            // Start queued transfer if content length is known and exceeds transferBuffer
            ESP_LOGI(TAG, "Starting queued send of %d bytes", content_length);

            // Set the queue and the size of the reponse in the response object
            command->response()->queue(client->queue(), (uint16_t) content_length);

            // Ask the command to send the SERVICE_RESPONSE_READY event so that
            // Service will start reading and sending data down to the C64 as soon as
            // it becomes available
            command->responseReady();

            // Start the queueing task that reads from the connection and inserts it at the end
            // of the queue in chunks of WIC64_QUEUE_ITEM_SIZE. We'll pass a pointer to the
            // content_length variable via the pvParameters argument.
            xTaskCreatePinnedToCore(queueTask, "SENDER", 4096, &content_length, 30, NULL, 1);
        }
        else { // Content-Length not send by server or Transfer-Encoding: chunked
            ESP_LOGI(TAG, "Reading response data of unknown length (limited to 64kb)");

            // Read up to 64kb from the connection into the static transfer buffer
            if ((size = esp_http_client_read(m_client, (char*) transferBuffer, 0x10000)) == -1) {
                ESP_LOGE(TAG, "Read Error");
                goto ERROR;
            }
            ESP_LOGI(TAG, "Read %d bytes", size);

            command->response()->wrap(transferBuffer, size);
        }

    DONE:
        // Send response unless already queued
        if(!command->response()->isQueued()) {
            command->responseReady();
        }
        return;

    ERROR:
        ESP_LOGW(TAG, "Sending error response");
        command->response()->wrap(error);
        closeConnection();
        goto DONE;
    }

    void Client::closeConnection(void) {
        if (m_client != NULL) {
            ESP_LOGW(TAG, "Closing connection");
            esp_err_t result;

            if ((result = esp_http_client_close(m_client)) != ESP_OK) {
                ESP_LOGE(TAG, "Error closing connection: %s", esp_err_to_name(result));
            }

            if ((result = esp_http_client_cleanup(m_client)) != ESP_OK) {
                ESP_LOGE(TAG, "Error cleaning up: %s", esp_err_to_name(result));
            }
            m_client = NULL;
        }
    }

    void Client::closeConnectionUnlessKeptAlive() {
        if (!m_keepAlive) {
            closeConnection();
        }
    }

    bool Client::isConnectionClosed() {
        return m_client == NULL;
    }

    void Client::queueTask(void *content_length_ptr) {
        int32_t content_length = *((int32_t *) content_length_ptr);
        static uint8_t data[WIC64_QUEUE_ITEM_SIZE];

        int32_t bytes_read = 0;
        int32_t total_bytes_read = 0;
        int16_t timeout_ms = 5000;
        ESP_LOGD(TAG, "Client queue task queueing %d bytes...", content_length);

        do {
            bytes_read = esp_http_client_read(client->handle(), (char*) data, WIC64_QUEUE_ITEM_SIZE);

            if (bytes_read == -1) {
                ESP_LOGE(TAG, "Read Error");
                break;
            }
            ESP_LOGV(TAG, "Queueing %d bytes", WIC64_QUEUE_ITEM_SIZE);
            if (xQueueSend(client->queue(), data, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
                ESP_LOGW(TAG, "Could not send to queue for more than %dms", timeout_ms);
                break;
            }
            total_bytes_read += bytes_read;

        } while (total_bytes_read < content_length);

        ESP_LOGV(TAG, "Queueing task deleting itself");
        vTaskDelete(NULL);
    }
}
