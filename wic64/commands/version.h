#ifndef WIC64_VERSION_H
#define WIC64_VERSION_H

#include "command.h"

namespace WiC64 {
    class Version : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_VERSION_H
