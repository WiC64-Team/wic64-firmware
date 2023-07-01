#include "webserver.h"
#include "utilities.h"
#include "version.h"

namespace WiC64 {
    extern Webserver *webserver;
    const char* Webserver::TAG = "SERVER";

    Webserver::Webserver() {
        m_arduinoWebServer = new WebServer(80);
        m_arduinoWebServer->on("/", log);
        m_arduinoWebServer->on("/log", log);
        m_arduinoWebServer->begin();
        ESP_LOGI(TAG, "Webserver listening on port 80");
    }

    void Webserver::serve(void)  {
        m_arduinoWebServer->handleClient();
    }

    void Webserver::reply(const String &response) {
        m_arduinoWebServer->send(200, "text/html", response);
    }

    const String &Webserver::header(const String &title) {
        static String header =
            "<html><head><title>WiC64 " + title + "</title></head>"
            "<body>"
            "<h2>WiC64 " + title + "</h2>"
            "<hr/>"
            "<p><small>Firmware Version " WIC64_VERSION_STRING "</small></p>"
            "<hr/>";
        return header;
    }

    void Webserver::log(void) {
        WebServer *arduinoWebServer = webserver->arduinoWebserver();
        String level;

        if (!arduinoWebServer->hasArg("level")) {
            goto PAGE;
        }

        level = arduinoWebServer->arg("level");

        if (level == "NONE"   ) WiC64::loglevel(ESP_LOG_NONE);    else
        if (level == "ERROR"  ) WiC64::loglevel(ESP_LOG_ERROR);   else
        if (level == "WARN"   ) WiC64::loglevel(ESP_LOG_WARN);    else
        if (level == "INFO"   ) WiC64::loglevel(ESP_LOG_INFO);    else
        if (level == "DEBUG"  ) WiC64::loglevel(ESP_LOG_DEBUG);   else
        if (level == "VERBOSE") WiC64::loglevel(ESP_LOG_VERBOSE); else goto ERROR;

    PAGE:
        webserver->reply(
            webserver->header("Loglevel") +

            "<p>Current level: <strong>" +
            log_level_to_string(esp_log_level_get(WiC64::TAG)) +
            "</strong></p>"

            "<ul>"
            "<li><a href='/log?level=NONE'>NONE</a></li>"
            "<li><a href='/log?level=ERROR'>ERROR</a></li>"
            "<li><a href='/log?level=WARN'>WARN</a></li>"
            "<li><a href='/log?level=INFO'>INFO</a></li>"
            "<li><a href='/log?level=DEBUG'>DEBUG</a></li>"
            "<li><a href='/log?level=VERBOSE'>VERBOSE</a></li>"
            "</ul>"

            + webserver->footer()
        );
        return;

    ERROR:
        webserver->reply("Usage: /log?level=<NONE|ERROR|WARN|INFO|DEBUG|VERBOSE>");
    }

    const String& Webserver::footer() {
        static const String footer = "</body></html>";
        return footer;
    }
}
