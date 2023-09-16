#ifndef WIC64_REBOOT_H
#define WIC64_REBOOT_H

#include "command.h"

namespace WiC64 {
    class Reboot : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_REBOOT_H
