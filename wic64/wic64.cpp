#include "wic64.h"
#include "settings.h"
#include "display.h"
#include "connection.h"
#include "httpClient.h"
#include "tcpClient.h"
#include "webserver.h"
#include "userport.h"
#include "service.h"
#include "settings.h"
#include "clock.h"
#include "led.h"
#include "buttons.h"
#include "utilities.h"
#include "commands/http.h"
#include "commands/scan.h"
#include "commands/connect.h"
#include "commands/time.h"
#include "commands/timezone.h"
#include "commands/tcp.h"

#include "esp_log.h"

using namespace WiC64;

namespace WiC64 {
    Userport   *userport;
    Service    *service;
    HttpClient *httpClient;
    TcpClient  *tcpClient;
    Settings   *settings;
    Display    *display;
    Connection *connection;
    Webserver  *webserver;
    Clock      *clock;
    Led        *led;
    Buttons    *buttons;

    const char* WiC64::TAG = "WIC64";

    uint8_t *transferBuffer;

    WiC64::WiC64() {
        transferBuffer = (uint8_t*) calloc(0x10000+1, sizeof(uint8_t));

        loglevel(ESP_LOG_INFO);
        ESP_LOGW(TAG, "Firmware version %s", WIC64_VERSION_STRING);

        userport   = new Userport();
        service    = new Service();
        httpClient = new HttpClient();
        tcpClient  = new TcpClient();
        settings   = new Settings();
        display    = new Display();
        connection = new Connection();
        webserver  = new Webserver();
        clock      = new Clock();
        led        = new Led();
        buttons    = new Buttons();

        settings->userportDisconnected()
            ? userport->disconnect()
            : userport->connect();

        if (!userport->isConnected()) {
            display->connected(false);
        }

        connection->connect();

        log_task_list(TAG, ESP_LOG_WARN);
        log_free_mem(TAG, ESP_LOG_WARN);

        loglevel(ESP_LOG_WARN);
    }

    void WiC64::loglevel(esp_log_level_t loglevel) {
        esp_log_level_set(WiC64::TAG, loglevel);
        esp_log_level_set(Userport::TAG, loglevel);
        esp_log_level_set(Service::TAG, loglevel);
        esp_log_level_set(Settings::TAG, loglevel);
        esp_log_level_set(Connection::TAG, loglevel);
        esp_log_level_set(Display::TAG, loglevel);
        esp_log_level_set(HttpClient::TAG, loglevel);
        esp_log_level_set(TcpClient::TAG, loglevel);
        esp_log_level_set(Webserver::TAG, loglevel);
        esp_log_level_set(Request::TAG, loglevel);
        esp_log_level_set(Clock::TAG, loglevel);
        esp_log_level_set(Command::TAG, loglevel);
        esp_log_level_set(Http::TAG, loglevel);
        esp_log_level_set(Scan::TAG, loglevel);
        esp_log_level_set(Connect::TAG, loglevel);
        esp_log_level_set(Time::TAG, loglevel);
        esp_log_level_set(Timezone::TAG, loglevel);
        esp_log_level_set(Buttons::TAG, loglevel);
    }
}