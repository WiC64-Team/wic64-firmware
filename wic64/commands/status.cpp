
#include "status.h"
#include "utilities.h"

namespace WiC64 {
    const char* Status::TAG = "STATUS";

    const char* Status::describe() {
        return "Status (get last command status)";
    }

    void Status::execute(void) {
        static char buffer[255];
        char* message = (char*) buffer;
        bool uppercase = request()->payload()->data()[0];

        strcpy(message, Command::statusMessage());

        if (uppercase) {
            lowercase(message);
        }

        ascii2petscii(message);
        response()->copyString(message);
        responseReady();
    }
}
