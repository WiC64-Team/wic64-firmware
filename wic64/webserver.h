#include "wic64.h"
#include "WebServer.h"

namespace WiC64 {
    class Webserver {
        public: static const char* TAG;

        private:
            WebServer *m_arduinoWebServer;
            WebServer *arduinoWebserver(void) { return m_arduinoWebServer; };
        public:
            Webserver();

            void serve(void);
            void reply(const String& response);

            const String& header(const String& title);
            const String& footer(void);

            static void log(void);
    };
}