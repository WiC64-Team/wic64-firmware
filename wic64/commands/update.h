#ifndef WIC64_UPDATE_H
#define WIC64_UPDATE_H

#include "command.h"

namespace WiC64 {
    class Update : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_UPDATE_H
