#ifndef WIC64_TCP_H
#define WIC64_TCP_H

#include "command.h"

namespace WiC64 {
    class Tcp : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_TCP_H
