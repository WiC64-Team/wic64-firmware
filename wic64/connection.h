#ifndef WIC64_CONNECTION_H
#define WIC64_CONNECTION_H

#include "WiFi.h"

namespace WiC64 {
    class Connection {
        public: static const char* TAG;

        private:
            static String getStoredSSID(void);
            static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onGotIp(WiFiEvent_t event, WiFiEventInfo_t info);

        public:
            Connection();

            bool isConnected(void);
            void connect(void);

            const char* ipAddress();
            const char* macAddress();
    };

}
#endif // WIC64_CONNECTION_H
