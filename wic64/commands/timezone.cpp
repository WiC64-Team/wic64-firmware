#include "timezone.h"
#include "clock.h"

namespace WiC64 {
    const char* Timezone::TAG = "TIMEZONE";

    extern Clock *clock;

    // Why is this a fixed array of gmt offsets for just a few select timezones?
    // REDESIGN: Use official posix timezones and allow the user to choose from a *complete* list
    const int32_t Timezone::timezones[32] = {
        0, 0, 3600, 7200, 7200, 10800, 12600, 14400,
        18000, 19800, 21600, 25200, 28800, 32400, 34200, 36000,
        39600, 43200, -39600, -36000, -32400, -28800, -25200,
        -25200, -21600, -18000, -18000, -14400, -12600, -10800,
        -10800, -3600,
    };

    const char* Timezone::describe() {
        return "Timezone (get/set timezone)";
    }

    void Timezone::execute(void) {
        switch (request()->id()) {
            case GET: get(); break;
            case SET: set(); break;
        }
        responseReady();
    }

    void Timezone::get(void) {
        // The current gmt offset in seconds is send to the C64
        // as a string containing the decimal representation.

        static char timezone[7];
        snprintf(timezone, 7, "%d", clock->timezone());
        response()->wrap(timezone);
    }

    void Timezone::set(void) {
        // The index into the array of gmt offsets is received
        // in two bytes, where the first byte contains the low
        // digit and the second byte contains the high digit.
        // Yes, seriously. I refuse to even validate this input.

        uint8_t low_digit = request()->argument()->data()[0];
        uint8_t high_digit = request()->argument()->data()[1];
        uint8_t index = high_digit * 10 + low_digit;
        clock->timezone(timezones[index]);
    }
}
