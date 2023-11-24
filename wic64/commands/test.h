#ifndef WIC64_ECHO_H
#define WIC64_ECHO_H

#include "command.h"
#include "data.h"

namespace WiC64 {
    class Test : public Command {
        public:
            using Command::Command;
            const char* describe();
            void execute(void);
    };
}
#endif // WIC64_ECHO_H
