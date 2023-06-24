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

    HttpGet::~HttpGet() {
        if (m_response != NULL) {
            delete m_response;
        }
    }

    void HttpGet::expandURL(String &url) {
        // replace "%mac" with the mac address, remove colons first
        if (url.indexOf("%mac") != -1) {
            ESP_LOGI(TAG, "Replacing \"%%mac\" with MAC address and adding security token");

            String mac(connection->macAddress());
            mac.replace(":", "");
            url.replace("%mac", mac + settings->securityToken());
        }
    }

    Data* HttpGet::execute(void) {
        String url((char*) request()->argument()->data());
        ESP_LOGI(TAG, "Received URL [%s]", url.c_str());

        expandURL(url);

        ESP_LOGI(TAG, "Fetching URL [%s]", url.c_str());
        m_response = client->get(url);

        if (isVersion1() && url.endsWith(".prg")) {
            m_response->sizeToReport(m_response->size() - 2);
        }

        return m_response;
    }
}
