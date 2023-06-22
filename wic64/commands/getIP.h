#ifndef WIC64_COMMAND_GET_IP
#define WIC64_COMMAND_GET_IP

#include "command.h"
#include "data.h"

namespace WiC64 {
    class GetIP : public Command {
        private:
            Data* m_response;

        public:
            using Command::Command;
            ~GetIP();

            Data* execute(void);
    };
}

 #endif // WIC64_COMMAND_GET_IP