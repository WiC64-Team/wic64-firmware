#include "http.h"
#include "client.h"
#include "connection.h"
#include "settings.h"
#include "utilities.h"

#include "WString.h"

namespace WiC64 {
    const char* Http::TAG = "HTTP";

    extern Client *client;
    extern Connection *connection;
    extern Settings *settings;

    void Http::execute(void) {
        request()->id() == 0x24 ? post() : get();
    }

    void Http::get(void) {
        String url = String(request()->argument()->c_str());

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        encode(url);
        sanitize(url);
        expand(url);
        analyze(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        client->get(this, url); // client will call responseReady()
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

        client->post(this, url, data); // client will call responseReady()

        delete data;
    }

    const char *Http::describe(void) {
        switch (request()->id()) {
            case 0x01: return "HTTP GET (fetch URL)"; break;
            case 0x0f: return "HTTP GET (fetch encoded URL)"; break;
            case 0x24: return "HTTP POST (post data to URL)"; break;
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

    // REDESIGN: Remove this hack, just POST binary data or use base64
    void Http::encode(String& url) {
        // If this command is executed via id 0x0f, encode HEX data
        // after "<$" markers in the request data using two lowercase
        // hex digits per byte. The marker is followed by a 16-bit little
        // endian value denoting the number of bytes to encode, followed
        // by the actual bytes.

        if (request()->id() != 0x0f) return;
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

        if (client->statusCode() == 201) {
            char key[256] = "";
            char value[256] = "";
            char reply[256] = "";

            strncpy(key, response()->field(1), 255);
            strncpy(value, response()->field(2), 255);
            strncpy(reply, response()->field(3), 255);

            ESP_LOGI(TAG, "Change of setting requested by sever: [%s] = [%s] => [%s]", key, value, reply);

            if (strcmp(key, "sectokenname") == 0) {
                settings->securityTokenKey(value);
            }
            else if (strcmp(key, settings->securityTokenKey().c_str()) == 0) {
                settings->securityToken(value);
            }
            else {
                ESP_LOGE(TAG, "Unknown setting: [%s]", key);
                response()->copy("!0");
                return;
            }
            response()->copy(reply);
        }
    }

    void Http::responseReady(void) {
        adjustResponseSizeForProgramFiles();
        handleSettingChangeRequestFromServer();

        Command::responseReady();
    }
}