#ifndef WIC64_SSID_H
#define WIC64_SSID_H

#include "command.h"

namespace WiC64 {
    class SSID : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_SSID_H
