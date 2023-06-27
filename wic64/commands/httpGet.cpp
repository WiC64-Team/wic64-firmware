#include "wic64.h"
#include "httpGet.h"
#include "client.h"
#include "connection.h"
#include "settings.h"
#include "utilities.h"

#include "WString.h"

namespace WiC64 {
    const char* HttpGet::TAG = "HTTPGET";

    extern Client *client;
    extern Connection *connection;
    extern Settings *settings;

    void HttpGet::analyze(const String &url) {
        m_isProgramFile = isVersion1() && url.endsWith(".prg");
    }

    // REDESIGN: Don't accept sloppy input
    void HttpGet::sanitize(String &url)
    {
        if (url.indexOf(" ") != -1) {
            ESP_LOGW(TAG, "Removing spaces from URL");
            url.replace(" ", "");
        }
    }

    // REDESIGN: Add file type in response header
    void HttpGet::expand(String& url) {
        if (url.indexOf("%mac") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%mac\" with MAC address and security token in URL");

            String mac(connection->macAddress());
            mac.replace(":", "");
            url.replace("%mac", mac + settings->securityToken());
        }
    }

    void HttpGet::execute(void) {
        String url = String((char*) request()->argument()->data());

        ESP_LOGD(TAG, "Received URL [%s]", url.c_str());

        analyze(url);
        sanitize(url);
        expand(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        client->get(this, url); // client will call responseReady()
    }

    void HttpGet::responseReady(void) {
        if (isProgramFile() && response()->size() > 2) {
            response()->sizeToReport(response()->size() - 2);
        }
        Command::responseReady();
    }
}
