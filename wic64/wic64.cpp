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
#include "protocol.h"
#include "protocols/legacy.h"
#include "protocols/standard.h"
#include "protocols/extended.h"
#include "command.h"
#include "commands/http.h"
#include "commands/scan.h"
#include "commands/connect.h"
#include "commands/configured.h"
#include "commands/connected.h"
#include "commands/time.h"
#include "commands/timezone.h"
#include "commands/tcp.h"
#include "commands/update.h"
#include "commands/reboot.h"
#include "commands/deprecated.h"
#include "commands/undefined.h"
#include "commands/status.h"
#include "commands/timeout.h"

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
    QueueHandle_t transferQueue;
    uint8_t transferQueueSendBuffer[WIC64_QUEUE_ITEM_SIZE];
    uint8_t transferQueueReceiveBuffer[WIC64_QUEUE_ITEM_SIZE];

    uint32_t transferTimeout = WIC64_DEFAULT_TRANSFER_TIMEOUT;
    uint32_t customTransferTimeout = 0;

    uint32_t remoteTimeout = WIC64_DEFAULT_REMOTE_TIMEOUT;
    uint32_t customRemoteTimeout = 0;

    WiC64::WiC64() {
        loglevel(ESP_LOG_INFO);
        ESP_LOGW(TAG, "Booting Firmware version %s", WIC64_VERSION_STRING);

        transferBuffer = (uint8_t*) calloc(0x10000+1, sizeof(uint8_t));

        if (transferBuffer == NULL) {
            ESP_LOGE(TAG, "Fatal: could not allocate transfer buffer");
            return;
        }

        transferQueue = xQueueCreateStatic(
            WIC64_QUEUE_SIZE,
            WIC64_QUEUE_ITEM_SIZE,
            transferBuffer,
            &m_staticQueue);

        if (transferQueue == NULL) {
            ESP_LOGE(TAG, "Fatal: could not allocate transfer buffer");
            return;
        }

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
            display->userportConnected(false);
        }

        if (connection->configured()) {
            connection->connect();
        }

        display->connectionConfigured(connection->configured());

        ESP_LOGI(TAG, "WiC64 initialized");
        log_free_mem(TAG, ESP_LOG_INFO);

        ESP_LOGW(TAG, "Switching to loglevel WARN");
        ESP_LOGW(TAG, "Visit http://<ip-address> to change loglevel");
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
        esp_log_level_set(Legacy::TAG, loglevel);
        esp_log_level_set(Standard::TAG, loglevel);
        esp_log_level_set(Extended::TAG, loglevel);
        esp_log_level_set(Command::TAG, loglevel);
        esp_log_level_set(Http::TAG, loglevel);
        esp_log_level_set(Scan::TAG, loglevel);
        esp_log_level_set(Connect::TAG, loglevel);
        esp_log_level_set(Configured::TAG, loglevel);
        esp_log_level_set(Connected::TAG, loglevel);
        esp_log_level_set(Time::TAG, loglevel);
        esp_log_level_set(Timezone::TAG, loglevel);
        esp_log_level_set(Buttons::TAG, loglevel);
        esp_log_level_set(Update::TAG, loglevel);
        esp_log_level_set(Reboot::TAG, loglevel);
        esp_log_level_set(Deprecated::TAG, loglevel);
        esp_log_level_set(Undefined::TAG, loglevel);
        esp_log_level_set(Status::TAG, loglevel);
        esp_log_level_set(Timeout::TAG, loglevel);
    }
}