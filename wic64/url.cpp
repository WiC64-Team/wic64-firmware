#include "url.h"
#include "connection.h"
#include "settings.h"

namespace WiC64 {
    const char* Url::TAG = "URL";

    extern Settings* settings;
    extern Connection* connection;

    bool Url::isHostWic64Net(void) {
        return startsWith("http://x.wic64.net") || startsWith("http://t.wic64.net");
    }

    bool Url::fetchesProgramFile(void) {
        return endsWith(".prg");
    }

    void Url::encode(Data* data) {
        // If this command is executed via id 0x0f, encode HEX data
        // after "<$" markers in the request data using two lowercase
        // hex digits per byte. The marker is followed by a 16-bit little
        // endian value denoting the number of bytes to encode, followed
        // by the actual bytes.

        if (indexOf("<$") == -1) return;

        const char* src = (char*) data->data();
        const uint32_t len = data->size();
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
        copy(encoded.c_str(), strlen(encoded.c_str()));
    }

    void Url::sanitize() {
        ESP_LOGW(TAG, "Removing spaces from URL");
        replace(" ", "");
    }

    void Url::expand() {
        if (indexOf("%ser") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%ser\" with configured server in URL");
            replace("%ser", settings->server());
        }

        if (startsWith("!")) {
            ESP_LOGI(TAG, "Replacing leading \"!\" with configured server in URL");
            replace("!", settings->server());
        }

        if (indexOf("%mac") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%mac\" with MAC address in URL");

            String mac(connection->macAddress());
            mac.replace(":", "");

            // REDESIGN: The whole session handling scheme is inherently insecure

            if (isHostWic64Net()) {
                // Append the security token (session id) to the mac address
                replace("%mac", mac + settings->securityToken());
            } else {
                replace("%mac", mac);
            }
        }
    }
}
