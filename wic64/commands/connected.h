#ifndef WIC64_CONNECTED_H
#define WIC64_CONNECTED_H

#include "command.h"

namespace WiC64 {
    class Connected : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_CONNECTED_H
