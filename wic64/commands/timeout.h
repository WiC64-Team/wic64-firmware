#ifndef WIC64_TIMEOUT_H
#define WIC64_TIMEOUT_H

#include "command.h"

namespace WiC64 {
    class Timeout : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_TIMEOUT_H
