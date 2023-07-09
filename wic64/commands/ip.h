#ifndef WIC64_IP_H
#define WIC64_IP_H

#include "command.h"
#include "data.h"

namespace WiC64 {
    class IP : public Command {
        public:
            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}

 #endif // WIC64_IP_H