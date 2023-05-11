#include "WiFi.h"

#include "connection.h"
#include "display.h"

extern Display *display;

Connection::Connection() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(("WiC64-" + WiFi.macAddress()).c_str());

    WiFi.onEvent(onConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(onDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(onGotIp, ARDUINO_EVENT_WIFI_STA_GOT_IP);

    connect();
}

void Connection::onConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    display->setSSID(WiFi.SSID());
    display->setRSSI(WiFi.RSSI());
    display->resetIp();
    display->setStatus("Connected");
}

void Connection::onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {  
    display->setSSID(WiFi.SSID());
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