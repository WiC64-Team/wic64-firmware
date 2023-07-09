#ifndef WIC64_CONNECT_H
#define WIC64_CONNECT_H

#include "command.h"

namespace WiC64 {
    class Connect : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            void execute(void);
    };
}
#endif // WIC64_CONNECT_H
