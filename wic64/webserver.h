#ifndef WIC64_WEBSERVER_H
#define WIC64_WEBSERVER_H

#include "wic64.h"

// Relative path required to build on windows, which does not support
// case-sensitive filenames
#include "../components/arduino/libraries/WebServer/src/WebServer.h"

namespace WiC64 {
    class Webserver {
        public: static const char* TAG;

        private:
            WebServer *m_arduinoWebServer;
            WebServer *arduinoWebserver(void) { return m_arduinoWebServer; };
            uint32_t loop_ms;
        public:
            Webserver();

            void loop(void);
            void reply(const String& response);
            void reloadAndClearQueryString(void);

            const String& header();
            const String& footer(void);

            static void request(void);
            static void wifi(void);

            static void disconnectTask(void*);
            static void factoryResetTask(void*);
    };
}

#endif // WIC64_WEBSERVER_H
