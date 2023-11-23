#ifndef WIC64_CONNECTION_H
#define WIC64_CONNECTION_H

#include "WiFi.h"

namespace WiC64 {
    class Connection {
        public: static const char* TAG;

        private:
            bool m_reconnecting = false;

            static String getStoredSSID(void);
            static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onGotIpAddress(WiFiEvent_t event, WiFiEventInfo_t info);

        public:
            Connection(void);
            bool reconnecting(void) { return m_reconnecting; }
            void reconnecting(bool reconnecting) { m_reconnecting = reconnecting; }

            const char* macAddress(void);
            const char* ipAddress(void);
            const char* SSID(void);
            const char* RSSI(void);

            uint16_t scanNetworks(void);

            void connect(void);
            void connect(const char* ssid, const char* passphrase);
            void disconnect(void);

            bool configured(void);
            bool connected(void);
            bool ipAddressAssigned(void);
            bool ready(void);

            void remove(void);
    };

}
#endif // WIC64_CONNECTION_H
