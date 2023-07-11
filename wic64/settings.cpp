#include "settings.h"

namespace WiC64 {
    const char* Settings::TAG = "SETTINGS";

    Settings::Settings() {
        m_preferences.begin("credentials");
    }

    String Settings::securityTokenKey(void) {
        return m_preferences.isKey(SECURITY_TOKEN_KEY)
            ? string(SECURITY_TOKEN_KEY)
            : String();
    }

    void Settings::securityTokenKey(const String& value) {
        string(SECURITY_TOKEN_KEY, value);
    }

    String Settings::securityToken(void) {
        return m_preferences.isKey(SECURITY_TOKEN_KEY)
            ? m_preferences.isKey(securityTokenKey().c_str())
                ? string(securityTokenKey().c_str())
                : String()
            : String();
    }

    void Settings::securityToken(const String& value) {
        if (m_preferences.isKey(SECURITY_TOKEN_KEY)) {
            if (!securityTokenKey().isEmpty()) {
                string(securityTokenKey().c_str(), value);
            }
        }
    }

    int32_t Settings::gmtOffsetSeconds(void) {
        return m_preferences.isKey(GMT_OFFSET_SECONDS_KEY)
            ? int32(GMT_OFFSET_SECONDS_KEY)
            : 0;
    }

    void Settings::gmtOffsetSeconds(int32_t value) {
        int32(GMT_OFFSET_SECONDS_KEY, value);
    }

    String Settings::string(const char* key) {
        return m_preferences.getString(key);
    }

    void Settings::string(const char *key, const String& value) {
        ESP_LOGI(TAG, "Setting %s = \"%s\"", key, value.c_str());
        m_preferences.putString(key, value);
    }

    bool Settings::boolean(const char *key) {
        return m_preferences.getBool(key);
    }

    void Settings::boolean(const char *key, const bool value) {
        ESP_LOGI(TAG, "Setting %s = %s", key, value ? "true" : "false");
        m_preferences.putBool(key, value);
    }

    int32_t Settings::int32(const char *key) {
        return m_preferences.getLong(key);
    }

    void Settings::int32(const char *key, const int32_t value) {
       ESP_LOGI(TAG, "Setting %s = %d", key, value);
       m_preferences.putLong(key, value);
    }
}
