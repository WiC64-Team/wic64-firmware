#include "clock.h"
#include "settings.h"

namespace WiC64 {
    const char* Clock::TAG = "CLOCK";

    extern Settings *settings;

    // Why is this a fixed array of gmt offsets for just a few select timezones?
    // REDESIGN: Use official posix timezones and allow the user to choose from a *complete* list
    const int32_t Clock::timezones[32] = {
        0, 0, 3600, 7200, 7200, 10800, 12600, 14400,
        18000, 19800, 21600, 25200, 28800, 32400, 34200, 36000,
        39600, 43200, -39600, -36000, -32400, -28800, -25200,
        -25200, -21600, -18000, -18000, -14400, -12600, -10800,
        -10800, -3600,
    };

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

        // reconfigure with current timezone setting
        // (may have changed in the meantime)
        reconfigure();

        if (getLocalTime(&tm)) {
            strftime(localTime, sizeof(localTime), "%H:%M:%S %d-%m-%Y", &tm);
            return localTime;
        } else {
            ESP_LOGE(TAG, "Failed to get local time");
        }
        return NULL;
    }
}