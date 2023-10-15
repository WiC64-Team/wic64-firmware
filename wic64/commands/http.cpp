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
        m_url = String(request()->payload()->c_str());

        ESP_LOGD(TAG, "Received URL [%s]", m_url.c_str());

        if (isEncoded()) {
            m_url.encode(request()->payload());
        }
        m_url.sanitize();
        m_url.expand();

        ESP_LOGI(TAG, "Fetching URL [%s]", m_url.c_str());
        httpClient->get(this, m_url); // client will call responseReady()
    }

    void Http::post(void) {
        uint8_t* payload = request()->payload()->data();
        uint32_t payload_size = request()->payload()->size();

        m_url = String((char*) payload);
        Data *data = new Data(payload + m_url.length() + 1, payload_size - m_url.length() - 1);

        ESP_LOGD(TAG, "Received URL [%s]", m_url.c_str());

        m_url.sanitize();
        m_url.expand();

        ESP_LOGI(TAG, "POSTing %d bytes to URL [%s]", data->size(), m_url.c_str());
        ESP_LOG_HEXV(TAG, "POST data", data->data(), data->size());

        httpClient->post(this, m_url, data); // client will call responseReady()

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

    bool Http::isEncoded(void) {
        return id() == WIC64_CMD_HTTP_GET_ENCODED;
    }

    void Http::lieAboutResponseSizeForProgramFile(void) {
        // For legacy requests only: If a cbm "program file" is requested, lie
        // about the size of the response data by subtracting 2. This has been
        // done in the original firmware to "simplify" the transfer routine for
        // loading files that contain a specific load address in the first two
        // bytes.

        if (isLegacyRequest() && m_url.fetchesProgramFile() && response()->size() > 2) {
            response()->sizeToReport(response()->size() - 2);
        }
    }

    void Http::sendPositiveResponseOnStatusCode201(void) {
        // For legacy requests only: The server may send a 201 status code with
        // a specially crafted response in order to ask the ESP to store certain
        // configuration values in flash. While this is now handled using
        // special HTTP headers, the existing client code still expects "00" as
        // confirmation of success.

        if (isLegacyRequest() && m_url.isHostWic64Net() && httpClient->statusCode() == 201) {
            response()->copyData("00");
        }
    }

    void Http::responseReady(void) {
        if (isLegacyRequest()) {
            lieAboutResponseSizeForProgramFile();
            sendPositiveResponseOnStatusCode201();
        }

        Command::responseReady();
    }
}
