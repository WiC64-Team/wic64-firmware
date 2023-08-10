#ifndef WIC64_COMMANDS_H
#define WIC64_COMMANDS_H

#include "command.h"
#include "version.h"
#include "http.h"
#include "ip.h"
#include "server.h"
#include "scan.h"
#include "connect.h"
#include "ssid.h"
#include "rssi.h"
#include "mac.h"
#include "time.h"
#include "timezone.h"
#include "echo.h"

namespace WiC64 {
    WIC64_COMMANDS = {
        WIC64_COMMAND(0x00, Version),
        WIC64_COMMAND(0x01, Http),
        WIC64_COMMAND(0x02, Connect),
        WIC64_COMMAND(0x06, IP),
        WIC64_COMMAND(0x08, Server),
        WIC64_COMMAND(0x0c, Scan),
        WIC64_COMMAND(0x0d, Connect),
        WIC64_COMMAND(0x0f, Http),
        WIC64_COMMAND(0x10, SSID),
        WIC64_COMMAND(0x11, RSSI),
        WIC64_COMMAND(0x12, Server),
        WIC64_COMMAND(0x14, MAC),
        WIC64_COMMAND(0x15, Time),
        WIC64_COMMAND(0x16, Timezone),
        WIC64_COMMAND(0x17, Timezone),
        WIC64_COMMAND(0x24, Http),
        WIC64_COMMAND(0x26, Version),
        WIC64_COMMAND(0xff, Echo),
    };
}

#endif // WIC64_COMMANDS_H