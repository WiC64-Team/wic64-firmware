#ifndef WIC64_TIME_H
#define WIC64_TIME_H

#include "command.h"

namespace WiC64 {
    class Time : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_TIME_H
