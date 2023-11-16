#ifndef WIC64_CONFIGURED_H
#define WIC64_CONFIGURED_H

#include "command.h"

namespace WiC64 {
    class Configured : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
            bool supportsProtocol(void);
    };
}
#endif // WIC64_CONFIGURED_H
