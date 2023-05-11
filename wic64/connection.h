#ifndef WIC64_CONNECTION_H
#define WIC64_CONNECTION_H

#include "WiFi.h"

class Connection {
    private:        
        static void onConnected(WiFiEvent_t event, WiFiEventInfo_t info);
        static void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
        static void onGotIp(WiFiEvent_t event, WiFiEventInfo_t info);

    public:
        Connection();
        bool isConnected(void);
        void connect(void);
};

#endif // WIC64_CONNECTION_H