#ifndef WIC64_STATUS_H
#define WIC64_STATUS_H

#include "command.h"

namespace WiC64 {
    class Status : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_STATUS_H
