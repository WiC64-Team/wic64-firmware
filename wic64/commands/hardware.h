#ifndef WIC64_HARDWARE_H
#define WIC64_HARDWARE_H

#include "command.h"

namespace WiC64 {
    class Hardware : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_HARDWARE_H
