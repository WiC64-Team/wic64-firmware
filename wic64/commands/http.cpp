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
        String url = String(request()->argument()->c_str());

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        encode(url);
        sanitize(url);
        expand(url);
        analyze(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        httpClient->get(this, url); // client will call responseReady()
    }

    void Http::post(void) {
        uint8_t* payload = request()->argument()->data();
        uint16_t payload_size = request()->argument()->size();
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

    // REDESIGN: Don't accept sloppy input
    void Http::sanitize(String &url) {
        if (url.indexOf(" ") != -1) {
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

        const char* src = (char*) request()->argument()->data();
        const uint16_t len = request()->argument()->size();
        String encoded = "";
        char code[3];

        for (uint16_t i=0; i<len; i++) {
            if (src[i] == '<' && src[i+1] == '$') {
                i += 2; // skip "<$"
                uint16_t data_size = (src[i+1]<<8) | src[i];

                i += 2; // skip size
                for (uint16_t k=0; k<data_size; k++, i++) {
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
        // If a cbm "program file" is requested, lie about the size of the
        // response data by subtracting 2. This has been done in the original
        // firmware to "simplify" the transfer routine for loading files that
        // contain a specific load address in the first two bytes.
        //
        // REDESIGN: Make the client side library responsible for dealing with
        // such cases, allow the user to specify whether the expected response
        // contains a load address.

        if (isProgramFile() && response()->size() > 2) {
            response()->sizeToReport(response()->size() - 2);
        }
    }

    void Http::handleSettingChangeRequestFromServer(void) {
        // When the server replies with HTTP status 201, the response contains
        // a request to change a specific setting in the ESP flash. The request
        // contains the setting key, value and a reply value that is sent to
        // the client instead of the server response.
        //
        // The original firmware allows values to be added and/or changed in the
        // ESP flash. Here we limit this behaviour to the "security token key" and
        // the actual "security token".
        //
        // REDESIGN: Have the server send custom headers for this purpose

        if (httpClient->statusCode() == 201) {
            static char key[256];
            static char value[256];
            static char reply[256];

            response()->field(1, key);
            response()->field(2, value);
            response()->field(3, reply);

            ESP_LOGI(TAG, "Change of setting requested by sever: [%s] = [%s] => [%s]", key, value, reply);

            if (strcmp(key, "sectokenname") == 0) {
                settings->securityTokenKey(value);
            }
            else if (strcmp(key, settings->securityTokenKey().c_str()) == 0) {
                settings->securityToken(value);
            }
            else {
                ESP_LOGE(TAG, "Unknown setting: [%s]", key);
                response()->copyData("!0");
                return;
            }
            response()->copyData(reply);
        }
    }

    void Http::responseReady(void) {
        if (isLegacyRequest()) {
            adjustResponseSizeForProgramFiles();
            handleSettingChangeRequestFromServer();
        }

        Command::responseReady();
    }
}
