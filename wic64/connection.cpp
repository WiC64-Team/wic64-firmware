#include "WiFi.h"
#include "esp_wifi.h"

#include "connection.h"
#include "display.h"

extern Display *display;

Connection::Connection(Display *display) {
    WiFi.setHostname(("WiC64-" + WiFi.macAddress()).c_str());

    WiFi.onEvent(onConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(onDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(onGotIp, ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.mode(WIFI_STA);
    connect();
}

String Connection::getStoredSSID(void) {
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config;
    esp_err_t esp_err;

    esp_wifi_init(&wifi_init_config);

    if ((esp_err = esp_wifi_get_config(WIFI_IF_STA, &wifi_config)) != ESP_OK) {
        log_e("esp_wifi_get_config() failed: %s", esp_err_to_name(esp_err));
        return "";
    }
    return String((char*)wifi_config.sta.ssid);
}

void Connection::onConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->setSSID(getStoredSSID());
    display->setRSSI(WiFi.RSSI());
    display->resetIp();
    display->setStatus("Connected");
}

void Connection::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->setSSID(getStoredSSID());
    display->setRSSI(WiFi.RSSI());
    display->resetIp();
    display->setStatus("Connecting...");

    WiFi.begin();
}

void Connection::onGotIp(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->setIp(WiFi.localIP().toString());
}

bool Connection::isConnected() {
    return WiFi.isConnected();
}

void Connection::connect() {
    WiFi.begin();
}