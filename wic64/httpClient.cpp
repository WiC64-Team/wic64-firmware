#include "wic64.h"
#include "httpClient.h"
#include "connection.h"
#include "utilities.h"
#include "settings.h"

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
    const char* HttpClient::TAG = "HTTPCLIENT";

    extern HttpClient* httpClient;
    extern Connection *connection;
    extern Settings *settings;

    HttpClient::HttpClient() {
        m_queue = xQueueCreate(WIC64_QUEUE_SIZE, WIC64_QUEUE_ITEM_SIZE);

        if (m_queue == NULL) {
            ESP_LOGE(TAG, "Failed to create queue");
        }
    }

    esp_err_t HttpClient::eventHandler(esp_http_client_event_t *event) {
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
                    httpClient->keepAlive(strcasecmp(event->header_value, "keep-alive") == 0);
                }

                // WiC64-Security-Token-Key: <key>
                if (strcmp(event->header_key, "WiC64-Security-Token-Key") == 0) {
                    settings->securityTokenKey(event->header_value);
                }

                // WiC64-Security-Token: <token>
                if (strcmp(event->header_key, "WiC64-Security-Token") == 0) {
                    settings->securityToken(event->header_value);
                }
                break;

            case HTTP_EVENT_ON_DATA:
                ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, size=%d", event->data_len);
                if (!connection_header_sent) {
                    httpClient->keepAlive(false);
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

    void HttpClient::get(Command *command, String& url) {
        request(command, HTTP_METHOD_GET, url, NULL);
    }

    void HttpClient::post(Command *command, String &url, Data *data) {
        request(command, HTTP_METHOD_POST, url, data);
    }

    void HttpClient::request(Command *command, esp_http_client_method_t method, String& url, Data* data) {
        int32_t size = 0;

        int32_t request_content_length = method == HTTP_METHOD_POST
            ? strlen(HEADER) + data->size() + strlen(FOOTER)
            : 0;

        m_statusCode = -1;

        // content_length needs to be static because it is eventually
        // passed by reference to the queue transfer task later on
        static int32_t content_length;

        int32_t result;

        // REDESIGN: This is the legacy error response. We need to send
        // qualified error information in the future.
        static char error[] = "!0";

        uint8_t retries = MAX_RETRIES;

        // The Arduino HTTPClient sends "ESP32HTTPClient" as the user-agent
        // Some programs test for this value, so we will mimic this behaviour
        // for compatibility.
        //
        // REDESIGN: Send "WiC64/<version>" as user-agent

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        esp_http_client_config_t config = {
            .url = url.c_str(),
            .user_agent = "ESP32HTTPClient",
            .method = method,
            .timeout_ms = 5 * 1000,
            .disable_auto_redirect = false,
            .max_redirection_count = 10,
            .event_handler = eventHandler,
            .buffer_size_tx = MAX_URL_LENGTH,
        };

        #pragma GCC diagnostic pop

        if (url.length() > MAX_URL_LENGTH) {
            ESP_LOGE(TAG, "URL length is limited to 2000 bytes");
            ESP_LOGE(TAG, "Please use HTTP POST to transfer large amounts of data");
            goto ERROR;
        }

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
            esp_http_client_set_method(m_client, method);

            if (esp_http_client_set_url(m_client, url.c_str()) == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to set URL");
                goto ERROR;
            };
        }

        if (method == HTTP_METHOD_POST) {
            esp_http_client_set_header(m_client, "Content-Type", "multipart/form-data;boundary=\"WiC64-Binary-Data\"");
        }

        if ((result = esp_http_client_open(m_client, request_content_length) != ESP_OK)) {
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

        if (method == HTTP_METHOD_POST) {
            ESP_LOGI(TAG, "Sending POST request body (%d bytes)", request_content_length);
            bool success =
                (esp_http_client_write(m_client, HEADER, strlen(HEADER)) == strlen(HEADER)) &&
                (esp_http_client_write(m_client, (const char*) data->data(), data->size()) == data->size()) &&
                (esp_http_client_write(m_client, FOOTER, strlen(FOOTER)) == strlen(FOOTER));

            if (!success) {
                ESP_LOGW(TAG, "Failed to send POST request body, retrying %d more time%s...",
                    retries, (retries > 1) ? "s" : "");

                if (retries-- > 0) {
                    closeConnection();
                    goto RETRY;
                } else {
                    goto ERROR;
                }
            }
        }

        ESP_LOGI(TAG, "Request sent, fetching response headers");

        if ((result = esp_http_client_fetch_headers(m_client)) == ESP_FAIL) {
            ESP_LOGW(TAG, "Failed to fetch headers, retrying %d more time%s...",
                retries, (retries > 1) ? "s" : "");

            if (retries-- > 0) {
                closeConnection();
                goto RETRY;
            } else {
                goto ERROR;
            }
        }

        content_length = result;
        m_statusCode = esp_http_client_get_status_code(m_client);

        ESP_LOGI(TAG, "HTTP %s Status = %d, content_length = %d",
            method == HTTP_METHOD_GET ? "GET" : "POST",
            m_statusCode,
            content_length);

        ESP_LOGI(TAG, "Keep alive: %s", m_keepAlive ? "true" : "false");

        // The client should handle redirects automatically, but this does not always seem to work,
        // see  https://www.esp32.com/viewtopic.php?t=26701 (no answers from Espressif yet)
        // If we end up with a redirection code here, try again manually up to MAX_RETRIES
        if (m_statusCode == 301 || m_statusCode == 302 || m_statusCode == 307 || m_statusCode == 308) {
            if (retries-- > 0) {
                ESP_LOGW(TAG, "Failed autoredirect (HTTP status %d), retrying manually %d more time%s...",
                    m_statusCode, retries, (retries > 1) ? "s" : "");

                esp_http_client_set_redirection(m_client);
                goto RETRY;
            } else {
                goto ERROR;
            }
        }

        // The previous firmware sends an error response for any status code != 200 or 201
        // There are more successful status codes, so we only send an error if code is >= 400
        if (m_statusCode >= 400) {
            ESP_LOGE(TAG, "Received HTTP status code %d >= 400, Sending error response", m_statusCode);
            goto ERROR;
        }

        if (content_length > 0xffff) {
            // Start queued transfer if content length is known and exceeds transferBuffer
            ESP_LOGI(TAG, "Starting queued send of %d bytes", content_length);

            // Set the queue and the size of the reponse in the response object
            command->response()->queue(httpClient->queue(), (uint16_t) content_length);

            // Ask the command to send the SERVICE_RESPONSE_READY event so that
            // Service will start reading and sending data down to the C64 as soon as
            // it becomes available
            command->responseReady();

            // Start the queueing task that reads from the connection and inserts it at the end
            // of the queue in chunks of WIC64_QUEUE_ITEM_SIZE. We'll pass a pointer to the
            // content_length variable via the pvParameters argument.
            xTaskCreatePinnedToCore(queueTask, "SENDER", 4096, &content_length, 30, NULL, 1);
        }
        else { // Content-Length <= 0xffff, not send by server or Transfer-Encoding: chunked
            if (content_length == 0) {
                ESP_LOGI(TAG, "Reading response data of unknown length (up to 64kb)");
            } else {
                ESP_LOGI(TAG, "Reading %d bytes of response data", content_length);
            }

            // Read up to 64kb from the connection into the static transfer buffer
            if ((size = esp_http_client_read(m_client, (char*) transferBuffer, 0xffff)) == -1) {
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

    void HttpClient::closeConnection(void) {
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

    void HttpClient::closeConnectionUnlessKeptAlive() {
        if (!m_keepAlive) {
            closeConnection();
        }
    }

    bool HttpClient::isConnectionClosed() {
        return m_client == NULL;
    }

    void HttpClient::queueTask(void *content_length_ptr) {
        int32_t content_length = *((int32_t *) content_length_ptr);
        static uint8_t data[WIC64_QUEUE_ITEM_SIZE];

        int32_t bytes_read = 0;
        int32_t total_bytes_read = 0;
        int16_t timeout_ms = 5000;
        ESP_LOGD(TAG, "Client queue task queueing %d bytes...", content_length);

        do {
            bytes_read = esp_http_client_read(httpClient->handle(), (char*) data, WIC64_QUEUE_ITEM_SIZE);

            if (bytes_read == -1) {
                ESP_LOGE(TAG, "Read Error");
                break;
            }
            ESP_LOGV(TAG, "Queueing %d bytes", WIC64_QUEUE_ITEM_SIZE);
            if (xQueueSend(httpClient->queue(), data, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
                ESP_LOGW(TAG, "Could not send to queue for more than %dms", timeout_ms);
                break;
            }
            total_bytes_read += bytes_read;

        } while (total_bytes_read < content_length);

        ESP_LOGV(TAG, "Queueing task deleting itself");
        vTaskDelete(NULL);
    }
}
