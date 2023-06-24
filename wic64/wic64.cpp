#include "wic64.h"
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

    Display *display;
    Connection *connection;
    Client *client;
    Service *service;
    Userport *userport;

    const char* WiC64::TAG = "WIC64";

    WiC64::WiC64() {
        log_level(ESP_LOG_DEBUG);
        ESP_LOGW(TAG, "Firmware version %s", WIC64_VERSION_STRING);

        display = new Display();
        connection = new Connection();
        client = new Client();
        service = new Service();
        userport = new Userport();

        connection->connect();
        userport->connect();
    }

    void WiC64::log_level(esp_log_level_t level) {
        esp_log_level_set(WiC64::TAG, level);
        esp_log_level_set(Userport::TAG, level);
        esp_log_level_set(Service::TAG, level);
        esp_log_level_set(Connection::TAG, level);
        esp_log_level_set(Display::TAG, level);
        esp_log_level_set(Client::TAG, level);
        esp_log_level_set(Request::TAG, level);
        esp_log_level_set(HttpGet::TAG, level);
    }
}