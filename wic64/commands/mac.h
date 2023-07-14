#ifndef WIC64_MAC_H
#define WIC64_MAC_H

#include "command.h"

namespace WiC64 {
    class MAC : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_MAC_H
