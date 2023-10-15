#include "commands.h"
#include "http.h"
#include "httpClient.h"
#include "connection.h"
#include "settings.h"
#include "utilities.h"

#include "WString.h"

namespace WiC64 {
    const char* Http::TAG = "HTTP";

    extern HttpClient *httpClient;
    extern Connection *connection;
    extern Settings *settings;

    void Http::execute(void) {
        if (!connection->ready()) {

            const char* message = !connection->connected()
                ? "WiFi not connected "
                : "No IP address assigned";

            ESP_LOGE(TAG, "Can't send HTTP request: %s", message);

            error(CONNECTION_ERROR, message, "!0");
            responseReady();
            return;
        }

        (id() == WIC64_CMD_HTTP_POST) ? post() : get();
    }

    void Http::get(void) {
        String url = String(request()->payload()->c_str());

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        encode(url);
        sanitize(url);
        expand(url);
        analyze(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        httpClient->get(this, url); // client will call responseReady()
    }

    void Http::post(void) {
        uint8_t* payload = request()->payload()->data();
        uint32_t payload_size = request()->payload()->size();
        size_t url_size = strlen((char*) payload);

        String url(payload, url_size);
        Data *data = new Data(payload + url_size + 1, payload_size - url_size - 1);

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        sanitize(url);
        expand(url);

        ESP_LOGI(TAG, "POSTing %d bytes to URL [%s]", data->size(), url.c_str());
        ESP_LOG_HEXV(TAG, "POST data", data->data(), data->size());

        httpClient->post(this, url, data); // client will call responseReady()

        delete data;
    }

    const char *Http::describe(void) {
        switch (id()) {
            case WIC64_CMD_HTTP_GET:
                return "HTTP GET (fetch URL)";

            case WIC64_CMD_HTTP_GET_ENCODED:
                return "HTTP GET (fetch encoded URL)";

            case WIC64_CMD_HTTP_POST:
                return "HTTP POST (post data to URL)";

            default: return "HTTP Request";
        }
    }

    void Http::analyze(const String &url) {
        m_isProgramFile = url.endsWith(".prg");
    }

    void Http::sanitize(String &url) {
        if (isLegacyRequest() && url.indexOf(" ") != -1) {
            ESP_LOGW(TAG, "Removing spaces from URL");
            url.replace(" ", "");
        }
    }

    void Http::expand(String& url) {
        if (url.indexOf("%mac") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%mac\" with MAC address and security token in URL");

            String mac(connection->macAddress());
            mac.replace(":", "");
            url.replace("%mac", mac + settings->securityToken());
        }

        if (url.indexOf("%ser") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%ser\" with configured server in URL");
            url.replace("%ser", settings->server());
        }

        if (url.startsWith("!")) {
            ESP_LOGI(TAG, "Replacing leading \"!\" with configured server in URL");
            url.replace("!", settings->server());
        }
    }

    void Http::encode(String& url) {
        // If this command is executed via id 0x0f, encode HEX data
        // after "<$" markers in the request data using two lowercase
        // hex digits per byte. The marker is followed by a 16-bit little
        // endian value denoting the number of bytes to encode, followed
        // by the actual bytes.

        if (id() != WIC64_CMD_HTTP_GET_ENCODED) return;
        if (url.indexOf("<$") == -1) return;

        const char* src = (char*) request()->payload()->data();
        const uint32_t len = request()->payload()->size();
        String encoded = "";
        char code[3];

        for (uint32_t i=0; i<len; i++) {
            if (src[i] == '<' && src[i+1] == '$') {
                i += 2; // skip "<$"
                uint32_t data_size = (src[i+1]<<8) | src[i];

                i += 2; // skip size
                for (uint32_t k=0; k<data_size; k++, i++) {
                    snprintf(code, 3, "%02x", src[i]);
                    encoded.concat(code, 2);
                }
                i -= 1;
            } else {
                encoded.concat(src[i]);
            }
        }
        url = encoded;
    }

    void Http::adjustResponseSizeForProgramFiles(void) {
        // For legacy requests only: If a cbm "program file" is requested, lie
        // about the size of the response data by subtracting 2. This has been
        // done in the original firmware to "simplify" the transfer routine for
        // loading files that contain a specific load address in the first two
        // bytes.

        if (isLegacyRequest() && isProgramFile() && response()->size() > 2) {
            response()->sizeToReport(response()->size() - 2);
        }
    }

    void Http::send201Response(void) {
        // For legacy requests only: The server may send a 201 status code with
        // a specially crafted response in order to ask the ESP to store certain
        // configuration values in flash. While this is now handled using
        // special HTTP headers, the existing client code still expects "00" as
        // confirmation of success.

        if (isLegacyRequest() && httpClient->statusCode() == 201 && response()->isEmpty()) {
            response()->copyData("00");
        }
    }

    void Http::responseReady(void) {
        if (isLegacyRequest()) {
            adjustResponseSizeForProgramFiles();
            send201Response();
        }

        Command::responseReady();
    }
}
