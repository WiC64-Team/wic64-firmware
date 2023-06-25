#include "WiFi.h"
#include "esp_wifi.h"

#include "connection.h"
#include "display.h"

namespace WiC64 {
    const char* Connection::TAG = "CONNECTION";

    extern Connection *connection;
    extern Display *display;

    Connection::Connection() {
        WiFi.mode(WIFI_STA);
        WiFi.setHostname(("WiC64-" + WiFi.macAddress()).c_str());

        WiFi.onEvent(onConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
        WiFi.onEvent(onDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        WiFi.onEvent(onGotIpAddress, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    }

    String Connection::getStoredSSID(void) {
        wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
        wifi_config_t wifi_config;
        esp_err_t esp_err;

        esp_wifi_init(&wifi_init_config);

        if ((esp_err = esp_wifi_get_config(WIFI_IF_STA, &wifi_config)) != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_get_config() failed: %s", esp_err_to_name(esp_err));
            return "";
        }
        return String((char*) wifi_config.sta.ssid);
    }

    void Connection::onConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
        ESP_LOGI(TAG, "WiFi connected");
        ESP_LOGI(TAG, "=> SSID: %s %ddbm", getStoredSSID().c_str(), WiFi.RSSI());

        display->setSSID(getStoredSSID());
        display->setRSSI(WiFi.RSSI());
        display->resetIp();
        display->setStatus("Connected");
    }

    void Connection::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
        ESP_LOGI(TAG, "WiFi connection lost");

        display->setSSID(getStoredSSID());
        display->setRSSI(WiFi.RSSI());
        display->resetIp();
        display->setStatus("Connecting...");

        connection->connect();
    }

    void Connection::onGotIpAddress(WiFiEvent_t event, WiFiEventInfo_t info) {
        ESP_LOGI(TAG, "=> ADDR: %s", connection->ipAddress());
        display->setIp(connection->ipAddress());
    }

    bool Connection::isConnected() {
        return WiFi.isConnected();
    }

    void Connection::connect() {
        ESP_LOGI(TAG, "Connecting to WiFi network...");
        WiFi.begin();
    }

    const char* Connection::ipAddress() {
        static char ip[16];
        strncpy(ip, WiFi.localIP().toString().c_str(), 16);
        return ip;
    }

    const char *Connection::macAddress() {
        static char mac[18];
        strncpy(mac, WiFi.macAddress().c_str(), 18);
        return mac;
    }
}
