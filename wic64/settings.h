#ifndef WIC64_SETTINGS_H
#define WIC64_SETTINGS_H

#include "wic64.h"
#include "Preferences.h"

namespace WiC64 {
    class Settings {
        private:
            Preferences m_preferences;

            String string(const char* key);
            void string(const char* key, const String& value);

            bool boolean(const char* key);
            void boolean(const char* key, const bool value);

            const char* SECURITY_TOKEN_KEY = "sectokenname";

        public:
            Settings();

            String securityTokenKey(void);
            void securityTokenKey(const String value);

            String securityToken(void);
            void securityToken(const String value);
    };
}

#endif // WIC64_SETTINGS_H