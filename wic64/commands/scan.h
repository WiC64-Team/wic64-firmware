#ifndef WIC64_SCAN_H
#define WIC64_SCAN_H

#include "command.h"

namespace WiC64 {
    class Scan : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            void execute(void);
    };
}
#endif // WIC64_SCAN_H
