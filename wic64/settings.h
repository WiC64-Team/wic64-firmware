#ifndef WIC64_SETTINGS_H
#define WIC64_SETTINGS_H

#include "wic64.h"
#include "Preferences.h"

namespace WiC64 {
    class Settings {
        public: static const char* TAG;

        private:
            Preferences m_preferences;

            String string(const char* key);
            void string(const char* key, const String& value);

            bool boolean(const char* key);
            void boolean(const char* key, const bool value);

            int32_t int32(const char* key);
            void int32(const char* key, const int32_t value);

            const char* DEFAULT_SERVER = "http://www.wic64.net/prg/";

            const char* SECURITY_TOKEN_KEY = "sectokenname";
            const char* SERVER_KEY = "server";
            const char* GMT_OFFSET_SECONDS_KEY = "gmtoffset";
            const char* LED_ENABLED_KEY = "killled";
            const char* DISPLAY_ROTATED_KEY = "displayrotate";
            const char* USERPORT_DISCONNECTED_KEY = "killswitch";

        public:
            Settings();

            String securityTokenKey(void);
            void securityTokenKey(const String& value);

            String securityToken(void);
            void securityToken(const String& value);

            String server(void);
            void server(const String& value);

            int32_t gmtOffsetSeconds(void);
            void gmtOffsetSeconds(int32_t value);

            bool ledEnabled(void);
            void ledEnabled(bool ledEnabled);

            bool displayRotated(void);
            void displayRotated(bool displayRotated);

            bool userportDisconnected(void);
            void userportDisconnected(bool userportDisconnected);
    };
}

#endif // WIC64_SETTINGS_H