#include "WiFi.h"
#include "esp_wifi.h"

#include "connection.h"
#include "display.h"

namespace WiC64 {
    const char* Connection::TAG = "CONNECTION";

    extern Connection *connection;
    extern Display *display;

    Connection::Connection() {
        WiFi.setHostname(("wic64-" + WiFi.macAddress()).c_str());
        WiFi.mode(WIFI_STA);

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
        ESP_LOGI(TAG, "SSID: %s %ddbm", getStoredSSID().c_str(), WiFi.RSSI());

        display->SSID(getStoredSSID());
        display->RSSI(WiFi.RSSI());
        display->ip("0.0.0.0");
        display->status("Connected");
    }

    void Connection::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
        ESP_LOGI(TAG, "WiFi not connected");

        display->SSID(getStoredSSID());
        display->RSSI(WiFi.RSSI());
        display->ip("0.0.0.0");

        if (connection->reconnecting()) {
            display->status("Reconnecting...");
            connection->connect();
        } else {
            display->status("Disconnected");
        }
    }

    void Connection::onGotIpAddress(WiFiEvent_t event, WiFiEventInfo_t info) {
        ESP_LOGI(TAG, "ADDR: %s", connection->ipAddress());
        ESP_LOGI(TAG, "MASK: %s", WiFi.subnetMask().toString().c_str());
        ESP_LOGI(TAG, "GATE: %s", WiFi.gatewayIP().toString().c_str());
        ESP_LOGI(TAG, "DNS1: %s", WiFi.dnsIP().toString().c_str());
        display->ip(connection->ipAddress());
    }

    void Connection::connect() {
        ESP_LOGI(TAG, "Connecting to WiFi network...");
        connection->reconnecting(true);
        WiFi.begin();
    }

    void Connection::disconnect(void) {
        reconnecting(false);
        WiFi.disconnect();
    }

    bool Connection::configured(void) {
        return !getStoredSSID().isEmpty();
    }

    bool Connection::connected(void)
    {
        return WiFi.isConnected();
    }

    bool Connection::ipAddressAssigned(void) {
        return WiFi.localIP() != IPADDR_NONE;
    }

    bool Connection::ready(void) {
        return connected() && ipAddressAssigned();
    }

    void Connection::connect(const char* ssid, const char* passphrase) {
        ESP_LOGI(TAG, "Connecting to SSID: [%s], Passphrase length: %d chars",
            ssid, strlen(passphrase));

        disconnect();
        WiFi.begin(ssid, passphrase);
    }

    const char *Connection::macAddress() {
        static char mac[MAC_STRLEN+1];
        stpncpy(mac, WiFi.macAddress().c_str(), MAC_STRLEN);
        mac[MAC_STRLEN] = '\0';
        return mac;
    }

    const char* Connection::ipAddress() {
        static char ip[MAX_IP_STRLEN+1];
        strncpy(ip, WiFi.localIP().toString().c_str(), MAX_IP_STRLEN);
        ip[MAX_IP_STRLEN] = '\0';
        return ip;
    }

    const char *Connection::SSID(void) {
        static char ssid[MAX_SSID_LEN+1];
        strncpy(ssid, getStoredSSID().c_str(), MAX_SSID_LEN);
        ssid[MAX_SSID_LEN] = '\0';
        return ssid;
    }

    const char *Connection::RSSI(void) {
        static char rssi[8+1];
        snprintf(rssi, 8+1, "%ddbm", WiFi.RSSI());
        return rssi;
    }

    uint16_t Connection::scanNetworks(void) {
        uint16_t num_networks;

        disconnect();
        num_networks = WiFi.scanNetworks();
        connect();

        return num_networks;
    }

    void Connection::remove(void) {
        wifi_config_t config;
        esp_wifi_get_config((wifi_interface_t)ESP_IF_WIFI_STA, &config);
        memset(config.sta.ssid, 0, sizeof(config.sta.ssid));
        memset(config.sta.password, 0, sizeof(config.sta.password));
        esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &config);
    }
}
