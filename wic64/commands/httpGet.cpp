#include "wic64.h"
#include "httpGet.h"
#include "client.h"
#include "connection.h"
#include "utilities.h"
#include "WString.h"

namespace WiC64 {

    extern Client *client;
    extern Connection *connection;

    HttpGet::~HttpGet() {
        if (m_response != NULL) {
            delete m_response;
        }
    }

    void HttpGet::expandURL(String &url) {
        // replace "%mac" with the mac address, remove colons first
        if (url.indexOf("%mac") != -1) {
            log_d("Replacing \"%%mac\" with MAC address and scurity token");
            String mac(connection->macAddress());
            mac.replace(":", "");
            url.replace("%mac", mac);
        }
    }

    Data* HttpGet::execute(void) {
        String url((char*) request()->argument()->data());
        log_d("Received URL [%s]", url.c_str());

        expandURL(url);

        log_d("Fetching URL [%s]", url.c_str());
        m_response = client->get(url);

        if (isVersion1() && url.endsWith(".prg")) {
            m_response->sizeToReport(m_response->size() - 2);
        }

        return m_response;
    }
}
