#ifndef WIC64_COMMANDS_H
#define WIC64_COMMANDS_H

#include "command.h"
#include "get.h"
#include "ip.h"
#include "scan.h"
#include "connect.h"
#include "echo.h"

namespace WiC64 {
    WIC64_COMMANDS = {
        WIC64_COMMAND(0x01, Get),
        WIC64_COMMAND(0x0f, Get),
        WIC64_COMMAND(0x06, IP),
        WIC64_COMMAND(0x0c, Scan),
        WIC64_COMMAND(0x0d, Connect),
        WIC64_COMMAND(0xff, Echo),
    };
}

#endif // WIC64_COMMANDS_H