
#include "time.h"
#include "clock.h"

namespace WiC64 {

    extern Clock *clock;

    const char* Time::TAG = "TIME";

    const char* Time::describe() {
        return "Time (get local time)";
    }

    void Time::execute(void) {
        const char* localTime = clock->localTime();

        if(localTime != NULL) {
            response()->wrap(localTime);
        }
        else {
            response()->copy("Could not get local time");
        }
        responseReady();
    }
}
