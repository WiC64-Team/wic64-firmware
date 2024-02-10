#include "wic64.h"
#include "httpClient.h"
#include "utilities.h"
#include "settings.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "WString.h"

namespace WiC64 {
    const char* HttpClient::TAG = "HTTPCLIENT";

    extern HttpClient* httpClient;
    extern Settings *settings;

    HttpClient::HttpClient() {
        ESP_LOGI(TAG, "HTTP client initialized");
    }

    esp_err_t HttpClient::eventHandler(esp_http_client_event_t *event) {
        switch(event->event_id) {
            case HTTP_EVENT_ERROR:
                if(event->data != NULL && event->data_len > 0) {
                    ESP_LOG_HEXV(TAG, "Error event data", (uint8_t*) event->data, event->data_len);
                }
                ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
                break;

            case HTTP_EVENT_ON_CONNECTED:
                ESP_LOGV(TAG, "HTTP_EVENT_ON_CONNECTED");
                break;

            case HTTP_EVENT_HEADER_SENT:
                ESP_LOGV(TAG, "HTTP_EVENT_HEADER_SENT");
                break;

            case HTTP_EVENT_ON_HEADER:
                ESP_LOGV(TAG, "HTTP_EVENT_ON_HEADER: %s: %s", event->header_key, event->header_value);

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
                ESP_LOGV(TAG, "HTTP_EVENT_ON_DATA, size=%d", event->data_len);
                break;

            case HTTP_EVENT_ON_FINISH:
                ESP_LOGV(TAG, "HTTP_EVENT_ON_FINISH");
                break;

            case HTTP_EVENT_DISCONNECTED:
                ESP_LOGV(TAG, "HTTP_EVENT_DISCONNECTED");
                break;
        }
        return ESP_OK;
    }

    void HttpClient::get(Command *command, String& url) {
        request(command, HTTP_METHOD_GET, url.c_str(), NULL);
    }

    void HttpClient::postUrl(String& url) {
        strncpy(m_postUrl, url.c_str(), MAX_URL_LENGTH);
    }

    void HttpClient::postData(Command *command, Data *data) {
        request(command, HTTP_METHOD_POST, m_postUrl, data);
    }

    void HttpClient::request(Command *command, esp_http_client_method_t method, const char* url, Data* data) {
        // content_length needs to be static because it is eventually
        // passed by reference to the queue transfer task later on
        static int32_t content_length;

        int32_t size = 0;
        int32_t result;

        int64_t request_content_length = method == HTTP_METHOD_POST
            ? strlen(HEADER) + data->size() + strlen(FOOTER)
            : 0;

        m_statusCode = -1;

        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

        esp_http_client_config_t config = {
            .url = url,
            .method = method,
            .timeout_ms = (int) remoteTimeout,
            .disable_auto_redirect = false,
            .max_redirection_count = 10,
            .event_handler = eventHandler,
            .buffer_size_tx = MAX_URL_LENGTH,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };

        #pragma GCC diagnostic pop

        if (strlen(url) == 0) {
            ESP_LOGE(TAG, "URL not specified");
            if (method == HTTP_METHOD_POST) {
                ESP_LOGE(TAG, "Set the URL beforehand using command 0x28 for POST requests");
            }
            command->error(Command::CLIENT_ERROR, "URL not specified", "!0");
            goto ERROR;
        }

        if (strlen(url) > MAX_URL_LENGTH) {
            ESP_LOGE(TAG, "URL length is limited to 2000 bytes");
            ESP_LOGE(TAG, "Please use HTTP POST to transfer large amounts of data");
            command->error(Command::CLIENT_ERROR, "URL too long (max 2000 bytes)", "!0");
            goto ERROR;
        }

        retries = MAX_RETRIES;
        timeRequestStarted = millis();

    RETRY:
        if (isConnectionClosed()) {
            ESP_LOGV(TAG, "Opening new connection");
            m_client = esp_http_client_init(&config);

            if (m_client == NULL) {
                ESP_LOGE(TAG, "Failed to create esp_http_client");
                command->error(Command::INTERNAL_ERROR, "Failed to create HTTP client", "!0");
                goto ERROR;
            }
        } else {
            esp_http_client_set_method(m_client, method);

            if (esp_http_client_set_url(m_client, url) == ESP_FAIL) {
                ESP_LOGE(TAG, "Failed to parse URL");
                command->error(Command::CLIENT_ERROR, "Malformed URL", "!0");
                goto ERROR;
            };
        }

        // The Arduino HTTPClient sends "ESP32HTTPClient" as the user-agent Some
        // existing programs test for this value, so for legacy requests, we
        // will mimic this behaviour for compatibility. For standard requests,
        // "WiC64/<version>" will be used.

        esp_http_client_set_header(m_client, "User-Agent",
            command->isLegacyRequest()
                ? "ESP32HTTPClient"
                : "WiC64/" WIC64_VERSION_SHORT_STRING " (ESP32)");

        if (method == HTTP_METHOD_POST) {
            esp_http_client_set_header(m_client, "Content-Type", "multipart/form-data;boundary=\"WiC64-Binary-Data\"");
        }

        if ((result = esp_http_client_open(m_client, request_content_length) != ESP_OK)) {
            ESP_LOGE(TAG, "Failed to open connection: %s", esp_err_to_name(result));

            ESP_LOGW(TAG, "Retrying %d more time%s...",
                retries+1, (retries > 1) ? "s" : "");

            if (canRetry(command)) {
                closeConnection();
                goto RETRY;
            } else {
                command->error(Command::NETWORK_ERROR, "Failed to open connection", "!0");
                goto ERROR;
            }
        }

        if (method == HTTP_METHOD_POST) {
            bool success = true;

            if (data->isQueued()) {
                ESP_LOGI(TAG, "Sending queued POST request body (%lld bytes)", request_content_length);

                uint32_t bytes_remaining = data->size();
                uint32_t items_remaining = WIC64_QUEUE_ITEMS_REQUIRED(bytes_remaining);
                uint16_t size = 0;

                // If sending the header fails, we can still retry
                if (!(success = esp_http_client_write(m_client, HEADER, strlen(HEADER))) == strlen(HEADER)) {
                    goto RETRY_POST;
                }

                // Once we've started to receive from the queue, we can't retry any more...
                while (items_remaining) {
                    size = items_remaining > 1
                        ? WIC64_QUEUE_ITEM_SIZE
                        : bytes_remaining;

                    if (!command->aborted() && xQueueReceive(data->queue(), transferQueueReceiveBuffer, pdMS_TO_TICKS(transferTimeout)) == pdTRUE) {

                        if (esp_http_client_write(m_client, (const char*) transferQueueReceiveBuffer, size) != size) {
                            ESP_LOGE(TAG, "Failed to send POST data to server");
                            command->error(Command::NETWORK_ERROR,
                                "Failed to send POST data to server", "!0");
                            goto ERROR;
                        }

                        bytes_remaining -= size;
                        items_remaining -= 1;
                    }
                    else {
                        ESP_LOGE(TAG, "Failed to receive POST data from client, aborting POST request");
                        // The client has already stopped sending data and the
                        // request has been finalized by the queueing task in
                        // Service, so it makes no sense to send an error
                        // response anymore
                        closeConnection();
                        return;
                    }
                }

                if (esp_http_client_write(m_client, FOOTER, strlen(FOOTER)) != strlen(FOOTER)) {
                    command->error(Command::NETWORK_ERROR,
                        "Failed to send POST body footer to server", "!0");
                    goto ERROR;
                }

            } else {
                ESP_LOGI(TAG, "Sending static POST request body (%lld bytes)", request_content_length);
                success =
                    (esp_http_client_write(m_client, HEADER, strlen(HEADER)) == strlen(HEADER)) &&
                    (esp_http_client_write(m_client, (const char*) data->data(), data->size()) == data->size()) &&
                    (esp_http_client_write(m_client, FOOTER, strlen(FOOTER)) == strlen(FOOTER));
            }

        RETRY_POST:
            if (!success) {
                ESP_LOGW(TAG, "Failed to send POST request body, retrying %d more time%s...",
                    retries+1, (retries > 1) ? "s" : "");

                if (canRetry(command) && !command->request()->payload()->isQueued()) {
                    closeConnection();
                    goto RETRY;
                } else {
                    command->error(Command::NETWORK_ERROR, "Failed to send POST data", "!0");
                    goto ERROR;
                }
            }
        }

        ESP_LOGI(TAG, "Request sent, fetching response headers");

        if ((result = esp_http_client_fetch_headers(m_client)) == ESP_FAIL) {

            if (canRetry(command) && !command->request()->payload()->isQueued()) {
                ESP_LOGW(TAG, "Failed to fetch headers, retrying %d more time%s...",
                    retries+1, (retries > 1) ? "s" : "");

                closeConnection();
                goto RETRY;
            } else {
                command->error(Command::SERVER_ERROR, "Failed to fetch response headers", "!0");
                goto ERROR;
            }
        }

        content_length = result;
        m_statusCode = esp_http_client_get_status_code(m_client);

        ESP_LOGI(TAG, "HTTP %s Status = %d, content_length = %d",
            method == HTTP_METHOD_GET ? "GET" : "POST",
            m_statusCode,
            content_length);

        // The client should handle redirects automatically, but this does not always seem to work,
        // see  https://www.esp32.com/viewtopic.php?t=26701 (no answers from Espressif yet)
        // If we end up with a redirection code here, try again manually up to MAX_RETRIES
        if (m_statusCode == 301 || m_statusCode == 302 || m_statusCode == 307 || m_statusCode == 308) {
            if (retries-- > 0 && !command->aborted()) {
                ESP_LOGW(TAG, "Failed autoredirect (HTTP status %d), retrying manually %d more time%s...",
                    m_statusCode, retries+1, (retries > 1) ? "s" : "");

                esp_http_client_set_redirection(m_client);
                goto RETRY;
            } else {
                command->error(Command::NETWORK_ERROR, "Failed to follow redirect", "!0");
                goto ERROR;
            }
        }

        // The previous firmware sends an error response for any status code != 200 or 201
        // There are more successful status codes, so we only send an error if code is >= 400
        if (m_statusCode >= 400) {
            ESP_LOGE(TAG, "Received HTTP status code %d >= 400", m_statusCode);
            command->error(Command::SERVER_ERROR, statusToString(m_statusCode), "!0");
            goto ERROR;
        }

        if (content_length >= 0x10000) {
            // Start queued transfer if content length is known and exceeds transferBuffer
            ESP_LOGI(TAG, "Starting queued send of %d bytes", content_length);

            // Set the queue and the size of the reponse in the response object
            command->response()->queue(transferQueue, (uint32_t) content_length);

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
                ESP_LOGE(TAG, "Read error");
                command->error(Command::NETWORK_ERROR, "Failed to read HTTP response", "!0");
                goto ERROR;
            }
            ESP_LOGI(TAG, "Read %d bytes", size);

            command->response()->set(transferBuffer, size);
        }

    DONE:
        // Send response unless already queued
        if(!command->response()->isQueued()) {
            command->responseReady();
        }
        return;

    ERROR:
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

    bool HttpClient::isConnectionClosed() {
        return m_client == NULL;
    }

    bool HttpClient::canRetry(Command* command) {
        uint32_t elapsed = millis() - timeRequestStarted;

        if (elapsed > (remoteTimeout - 1000)) {
            return false;
        }

        if (retries - 1 > 0 && !command->aborted()) {
            retries -= 1;
            return true;
        }

        return false;
    }

    void HttpClient::queueTask(void *content_length_ptr) {
        int32_t content_length = *((int32_t *) content_length_ptr);

        int32_t bytes_read = 0;
        int32_t total_bytes_read = 0;
        ESP_LOGD(TAG, "Client queue task queueing %d bytes...", content_length);

        do {
            bytes_read = esp_http_client_read(httpClient->handle(), (char*) transferQueueSendBuffer, WIC64_QUEUE_ITEM_SIZE);

            if (bytes_read == -1) {
                ESP_LOGE(TAG, "Read Error");
                httpClient->closeConnection();
                break;
            }
            ESP_LOGV(TAG, "Queueing %d bytes", WIC64_QUEUE_ITEM_SIZE);
            if (xQueueSend(transferQueue, transferQueueSendBuffer, pdMS_TO_TICKS(transferTimeout)) != pdTRUE) {
                ESP_LOGW(TAG, "Could not send to queue for more than %dms", transferTimeout);
                httpClient->closeConnection();
                break;
            }
            total_bytes_read += bytes_read;

        } while (total_bytes_read < content_length);

        ESP_LOGV(TAG, "Queueing task deleting itself");
        vTaskDelete(NULL);
    }

    const char *HttpClient::statusToString(int32_t code)
    {
        static char unknown[24];
        switch (code) {
            //####### 1xx - Informational #######
            case 100: return "100 Continue";
            case 101: return "101 Switching Protocols";
            case 102: return "102 Processing";
            case 103: return "103 Early Hints";

            //####### 2xx - Successful #######
            case 200: return "200 OK";
            case 201: return "20Created";
            case 202: return "201 Acepted";
            case 203: return "203 Non-Authoritative Information";
            case 204: return "204 No Content";
            case 205: return "205 Reset Content";
            case 206: return "206 Partial Content";
            case 207: return "207 Multi-Status";
            case 208: return "208 Already Reported";
            case 226: return "226 IM Used";

            //####### 3xx - Redirection #######
            case 300: return "300 Multiple Choices";
            case 301: return "301 Moved Permanently";
            case 302: return "302 Found";
            case 303: return "303 See Other";
            case 304: return "304 Not Modified";
            case 305: return "305 Use Proxy";
            case 307: return "307 Temporary Redirect";
            case 308: return "308 Permanent Redirect";

            //####### 4xx - Client Error #######
            case 400: return "400 Bad Request";
            case 401: return "401 Unauthorized";
            case 402: return "402 Payment Required";
            case 403: return "403 Forbidden";
            case 404: return "404 Not Found";
            case 405: return "405 Method Not Allowed";
            case 406: return "406 Not Acceptable";
            case 407: return "407 Proxy Authentication Required";
            case 408: return "408 Request Timeout";
            case 409: return "409 Conflict";
            case 410: return "410 Gone";
            case 411: return "411 Length Required";
            case 412: return "412 Precondition Failed";
            case 413: return "413 Content Too Large";
            case 414: return "414 URI Too Long";
            case 415: return "415 Unsupported Media Type";
            case 416: return "416 Range Not Satisfiable";
            case 417: return "417 Expectation Failed";
            case 418: return "418 I'm a teapot";
            case 421: return "421 Misdirected Request";
            case 422: return "422 Unprocessable Content";
            case 423: return "423 Locked";
            case 424: return "424 Failed Dependency";
            case 425: return "425 Too Early";
            case 426: return "426 Upgrade Required";
            case 428: return "428 Precondition Required";
            case 429: return "429 Too Many Requests";
            case 431: return "431 Request Header Fields Too Large";
            case 451: return "451 Unavailable For Legal Reasons";

            //####### 5xx - Server Error #######
            case 500: return "500 Internal Server Error";
            case 501: return "501 Not Implemented";
            case 502: return "502 Bad Gateway";
            case 503: return "503 Service Unavailable";
            case 504: return "504 Gateway Timeout";
            case 505: return "505 HTTP Version Not Supported";
            case 506: return "506 Variant Also Negotiates";
            case 507: return "507 Insufficient Storage";
            case 508: return "508 Loop Detected";
            case 510: return "510 Not Extended";
            case 511: return "511 Network Authentication Required";

            default:
                snprintf(unknown, 24, "Invalid HTTP status %d", code);
                return unknown;
        }
    }
}
