#include "webserver.h"
#include "connection.h"
#include "utilities.h"

namespace WiC64 {
    const char* Webserver::TAG = "SERVER";

    extern Webserver *webserver;
    extern Connection *connection;

    Webserver::Webserver() {
        m_arduinoWebServer = new WebServer(80);
        m_arduinoWebServer->on("/", request);
        m_arduinoWebServer->begin();
        ESP_LOGI(TAG, "Webserver initialized, listening on port 80");
    }

    void Webserver::serve(void)  {
        m_arduinoWebServer->handleClient();
    }

    void Webserver::reply(const String &response) {
        m_arduinoWebServer->send(200, "text/html", response);
    }

    void Webserver::reloadAndClearQueryString(void) {
        reply("<script>document.location.href='/';</script>");
    }

    const String &Webserver::header() {
        static String header =
            "<html><head><title>" WIC64_VERSION_STRING "</title></head>"
            "<body>"
            "<h3>" WIC64_VERSION_STRING "</h3>";
        return header;
    }

    void Webserver::request(void) {
        WebServer *server = webserver->arduinoWebserver();
        String level;
        String wifi;

        if (server->hasArg("level")) {
            level = server->arg("level");
            if (level == "NONE"   ) WiC64::loglevel(ESP_LOG_NONE);    else
            if (level == "ERROR"  ) WiC64::loglevel(ESP_LOG_ERROR);   else
            if (level == "WARN"   ) WiC64::loglevel(ESP_LOG_WARN);    else
            if (level == "INFO"   ) WiC64::loglevel(ESP_LOG_INFO);    else
            if (level == "DEBUG"  ) WiC64::loglevel(ESP_LOG_DEBUG);   else
            if (level == "VERBOSE") WiC64::loglevel(ESP_LOG_VERBOSE); else goto ERROR;
        }

        if (server->hasArg("disconnect")) {
            webserver->reloadAndClearQueryString();
            xTaskCreatePinnedToCore(disconnectTask, "DISCONNECT", 4096, NULL, 5, NULL, 0);
            return;
        }

        webserver->reply(
            webserver->header() +

            "<p>Current loglevel: <strong>" +
            log_level_to_string(esp_log_level_get(WiC64::TAG)) +
            "</strong></p>"

            "<ul>"
            "<li><a href='/?level=NONE'>NONE</a></li>"
            "<li><a href='/?level=ERROR'>ERROR</a></li>"
            "<li><a href='/?level=WARN'>WARN</a></li>"
            "<li><a href='/?level=INFO'>INFO</a></li>"
            "<li><a href='/?level=DEBUG'>DEBUG</a></li>"
            "<li><a href='/?level=VERBOSE'>VERBOSE</a></li>"
            "</ul>"

            "<p><a href='/?disconnect=1'>Disconnect WiFi</a><br/><small>(reset ESP to reconnect)</small></p>"

            + webserver->footer()
        );
        return;

    ERROR:
        webserver->reply("Usage: /?level=&lt;NONE|ERROR|WARN|INFO|DEBUG|VERBOSE&gt;");
    }

    const String& Webserver::footer() {
        static const String footer = "</body></html>";
        return footer;
    }

    void Webserver::disconnectTask(void*) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        connection->disconnect();
        vTaskDelete(NULL);
    }
}
