#ifndef WIC64_COMMAND_ECHO_H
#define WIC64_COMMAND_ECHO_H

#include "command.h"

namespace WiC64 {
    class Echo : public Command {
        public:
            using Command::Command;
            Data* execute(void);
    };
}
#endif // WIC64_COMMAND_ECHO_H