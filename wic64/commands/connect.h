#ifndef WIC64_CONNECT_H
#define WIC64_CONNECT_H

#include "command.h"

#include "WString.h"

namespace WiC64 {
    class Connect : public Command {
        public: static const char* TAG;

        private:
            const String ssid();

        public:
            static const uint8_t SSID_PASSED_AS_STRING = 0x02;
            static const uint8_t SSID_PASSED_VIA_INDEX = 0x0d;

            using Command::Command;
            const char* describe();
            void execute(void);
    };
}
#endif // WIC64_CONNECT_H
