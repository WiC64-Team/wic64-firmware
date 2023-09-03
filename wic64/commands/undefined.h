#ifndef WIC64_UNDEFINED_H
#define WIC64_UNDEFINED_H

#include "command.h"

namespace WiC64 {
    class Undefined : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_UNDEFINED_H
