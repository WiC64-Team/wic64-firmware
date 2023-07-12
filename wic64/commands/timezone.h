#ifndef WIC64_TIMEZONE_H
#define WIC64_TIMEZONE_H

#include "command.h"

namespace WiC64 {
    class Timezone : public Command {
        public: static const char* TAG;

        private:
            static const int32_t timezones[];

        public:
            static const uint8_t GET = 0x17;
            static const uint8_t SET = 0x16;

            using Command::Command;
            const char* describe(void);
            void execute(void);
            void get(void);
            void set(void);
    };
}
#endif // WIC64_TIMEZONE_H
