#include "settings.h"

namespace WiC64 {
    const char* Settings::TAG = "SETTINGS";

    Settings::Settings() {
        m_preferences.begin("credentials");
        ESP_LOGI(TAG, "Settings initialized");
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

    String Settings::server(void) {
        if (!m_preferences.isKey(SERVER_KEY) || string(SERVER_KEY).isEmpty()) {
            return DEFAULT_SERVER;
        }
        return string(SERVER_KEY);
    }

    void Settings::server(const String& value) {
        string(SERVER_KEY, value);
    }

    int32_t Settings::gmtOffsetSeconds(void) {
        return m_preferences.isKey(GMT_OFFSET_SECONDS_KEY)
            ? int32(GMT_OFFSET_SECONDS_KEY)
            : 0;
    }

    void Settings::gmtOffsetSeconds(int32_t value) {
        int32(GMT_OFFSET_SECONDS_KEY, value);
    }

    bool Settings::ledEnabled(void) {
        return m_preferences.isKey(LED_ENABLED_KEY)
            ? boolean(LED_ENABLED_KEY)
            : true;
    }

    void Settings::ledEnabled(bool ledEnabled) {
        boolean(LED_ENABLED_KEY, ledEnabled);
    }

    bool Settings::displayRotated(void) {
        return m_preferences.isKey(DISPLAY_ROTATED_KEY)
            ? boolean(DISPLAY_ROTATED_KEY)
            : false;
    }

    void Settings::displayRotated(bool displayRotated) {
        boolean(DISPLAY_ROTATED_KEY, displayRotated);
    }

    bool Settings::userportDisconnected(void) {
        return m_preferences.isKey(USERPORT_DISCONNECTED_KEY)
            ? boolean(USERPORT_DISCONNECTED_KEY)
            : false;
    }

    void Settings::userportDisconnected(bool userportDisconnected) {
        boolean(USERPORT_DISCONNECTED_KEY, userportDisconnected);
    }

    bool Settings::rebooting(void) {
        return m_preferences.isKey(REBOOTING_KEY)
            ? boolean(REBOOTING_KEY)
            : false;
    }

    void Settings::rebooting(bool rebooted) {
        boolean(REBOOTING_KEY, rebooted);
    }

    String Settings::string(const char* key) {
        return m_preferences.getString(key);
    }

    void Settings::string(const char *key, const String& value) {
        ESP_LOGI(TAG, "Setting %s = \"%s\"", key, value.c_str());
        if (m_preferences.getString(key) != value) {
            m_preferences.putString(key, value);
        }
    }

    bool Settings::boolean(const char *key) {
        return m_preferences.getBool(key);
    }

    void Settings::boolean(const char *key, const bool value) {
        ESP_LOGI(TAG, "Setting %s = %s", key, value ? "true" : "false");
        if (m_preferences.getBool(key) != value) {
            m_preferences.putBool(key, value);
        }
    }

    int32_t Settings::int32(const char *key) {
        return m_preferences.getLong(key);
    }

    void Settings::int32(const char *key, const int32_t value) {
        ESP_LOGI(TAG, "Setting %s = %d", key, value);
        if (m_preferences.getLong(key) != value) {
            m_preferences.putLong(key, value);
        }
    }
}
