#ifndef WIC64_COMMAND_GET_IP
#define WIC64_COMMAND_GET_IP

#include "command.h"
#include "data.h"

namespace WiC64 {
    class GetIP : public Command {
        public:
            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}

 #endif // WIC64_COMMAND_GET_IP