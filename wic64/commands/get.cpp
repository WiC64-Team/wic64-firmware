#include "get.h"
#include "client.h"
#include "connection.h"
#include "settings.h"
#include "utilities.h"

#include "WString.h"

namespace WiC64 {
    const char* Get::TAG = "HTTPGET";

    extern Client *client;
    extern Connection *connection;
    extern Settings *settings;

    void Get::analyze(const String &url) {
        m_isProgramFile = url.endsWith(".prg");
    }

    // REDESIGN: Don't accept sloppy input
    void Get::sanitize(String &url)
    {
        if (url.indexOf(" ") != -1) {
            ESP_LOGW(TAG, "Removing spaces from URL");
            url.replace(" ", "");
        }
    }

    // REDESIGN: Add file type in response header
    void Get::expand(String& url) {
        if (url.indexOf("%mac") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%mac\" with MAC address and security token in URL");

            String mac(connection->macAddress());
            mac.replace(":", "");
            url.replace("%mac", mac + settings->securityToken());
        }
    }

    const char *Get::describe(void) {
        return "HTTP GET (fetch URL)";
    }

    // REDESIGN: Remove this hack, just POST binary data
    void Get::encode(String& url) {
        // If this command has id 0x0f, encode HEX data after the
        // marker "$<" in the request data using two lowercase hex
        // digits per byte. This was required in the original firmware
        // for reasons I still don't quite understand.

        int16_t start;

        if (request()->id() == 0x0f && (start = url.indexOf("<$")) != -1) {
            // remove the data marker and the rest of the data that made
            // it into the url String before the first nullbyte occured
            url = url.substring(0, start);

            // get a pointer to the start of the data in the original request
            uint8_t* data = request()->argument()->data() + start + 2;

            // the first two bytes contain the size of the data
            uint16_t size = (*((uint16_t*) data));

            // skip ahead to the actual data
            data += 2;

            // use snprintf into a temporary char* and append it to the url
            // String  we'll later pass to client.get()
            char tmp[3];

            for (uint16_t i=0; i<size; i++) {
                snprintf(tmp, 3, "%02x", data[i]);
                url += tmp;
            }
        }
    }

    void Get::execute(void) {
        String url = String(request()->argument()->c_str());

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        encode(url);
        sanitize(url);
        expand(url);
        analyze(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        client->get(this, url); // client will call responseReady()
    }

    void Get::responseReady(void) {
        if (isVersion1() && isProgramFile() && response()->size() > 2) {
            response()->sizeToReport(response()->size() - 2);
        }
        Command::responseReady();
    }
}
