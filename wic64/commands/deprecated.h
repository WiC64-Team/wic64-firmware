#ifndef WIC64_DEPRECATED_H
#define WIC64_DEPRECATED_H

#include "command.h"

namespace WiC64 {
    class Deprecated : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            const char* reason(void);
            void execute(void);
    };
}
#endif // WIC64_DEPRECATED_H
