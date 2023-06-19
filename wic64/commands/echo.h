#ifndef WIC64_COMMAND_ECHO_H
#define WIC64_COMMAND_ECHO_H

#include "command.h"

class Echo : public Command {
    public:
        using Command::Command;
        Service::Data* execute(void);
};

#endif // WIC64_COMMAND_ECHO_H