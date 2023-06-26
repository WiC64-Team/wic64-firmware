#include "wic64.h"
#include "settings.h"
#include "display.h"
#include "connection.h"
#include "client.h"
#include "userport.h"
#include "service.h"
#include "utilities.h"
#include "version.h"
#include "commands/httpGet.h"

#include "esp_log.h"

using namespace WiC64;

namespace WiC64 {
    Settings *settings;
    Display *display;
    Connection *connection;
    Client *client;
    Service *service;
    Userport *userport;

    const char* WiC64::TAG = "WIC64";

    WiC64::WiC64() {
        loglevel(ESP_LOG_INFO);
        ESP_LOGW(TAG, "Firmware version %s", WIC64_VERSION_STRING);

        settings = new Settings();
        display = new Display();
        connection = new Connection();
        client = new Client();
        service = new Service();
        userport = new Userport();

        connection->connect();
        userport->connect();

        log_free_mem(TAG, ESP_LOG_WARN);
    }

    void WiC64::loglevel(esp_log_level_t level) {
        esp_log_level_set(WiC64::TAG, level);
        esp_log_level_set(Userport::TAG, level);
        esp_log_level_set(Service::TAG, level);
        esp_log_level_set(Connection::TAG, level);
        esp_log_level_set(Display::TAG, level);
        esp_log_level_set(Client::TAG, level);
        esp_log_level_set(Request::TAG, level);
        esp_log_level_set(Command::TAG, level);
        esp_log_level_set(HttpGet::TAG, level);
    }
}