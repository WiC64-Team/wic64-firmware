#ifndef WIC64_CONNECTION_H
#define WIC64_CONNECTION_H

#include "WiFi.h"
#include "display.h"

namespace WiC64 {
    class Connection {
        private:
            static String getStoredSSID(void);
            static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void onGotIp(WiFiEvent_t event, WiFiEventInfo_t info);

        public:
            Connection(Display *display);

            bool isConnected(void);
            void connect(void);

            const char* getIP();
    };

}
#endif // WIC64_CONNECTION_H