#include "clock.h"
#include "settings.h"

namespace WiC64 {
    const char* Clock::TAG = "CLOCK";

    extern Settings *settings;

    Clock::Clock() {
        configure();
    }

    void Clock::configure(void) {
        configTime(settings->gmtOffsetSeconds(), 3600, "pool.ntp.org");
    }

    void Clock::reconfigure(void) {
        configure();
    }

    const char* Clock::localTime(void) {
        static char localTime[25];
        struct tm tm;

        if (getLocalTime(&tm)) {
            strftime(localTime, sizeof(localTime), "%H:%M:%S %d-%m-%Y", &tm);
            return localTime;
        } else {
            ESP_LOGE(TAG, "Failed to get local time");
        }
        return NULL;
    }

    int32_t Clock::timezone(void) {
        return settings->gmtOffsetSeconds();
    }

    void Clock::timezone(int32_t timezone) {
        settings->gmtOffsetSeconds(timezone);
        reconfigure();
    }
}