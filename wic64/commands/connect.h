#ifndef WIC64_CONNECT_H
#define WIC64_CONNECT_H

#include "command.h"

#include "WString.h"

namespace WiC64 {
    class Connect : public Command {
        public: static const char* TAG;

        private:
            const char* ssid();
            const char* passphrase();

        public:
            using Command::Command;
            const char* describe();
            void execute(void);
    };
}
#endif // WIC64_CONNECT_H
