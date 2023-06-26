#ifndef WIC64_ECHO_H
#define WIC64_ECHO_H

#include "command.h"
#include "data.h"

namespace WiC64 {
    class Echo : public Command {
        public:
            using Command::Command;
            void execute(void);
    };
}
#endif // WIC64_ECHO_H
