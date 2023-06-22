#ifndef WIC64_COMMANDS_H
#define WIC64_COMMANDS_H

#include "command.h"
#include "getIP.h"
#include "echo.h"
namespace WiC64 {
    WIC64_COMMANDS = {
        WIC64_COMMAND(0x06, GetIP),
        WIC64_COMMAND(0xff, Echo),
    };
}

#endif // WIC64_COMMANDS_H