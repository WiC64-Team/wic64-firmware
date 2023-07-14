#ifndef WIC64_RSSI_H
#define WIC64_RSSI_H

#include "command.h"

namespace WiC64 {
    class RSSI : public Command {
        public:
            static const char* TAG;

            using Command::Command;
            const char* describe(void);
            void execute(void);
    };
}
#endif // WIC64_RSSI_H
