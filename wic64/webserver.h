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
            void reloadAndClearQueryString(void);

            const String& header();
            const String& footer(void);

            static void request(void);
            static void wifi(void);

            static void disconnectTask(void*);
    };
}