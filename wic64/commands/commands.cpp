#include "commands.h"
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
#include "tcp.h"
#include "echo.h"

namespace WiC64 {
    WIC64_COMMANDS = {
        WIC64_COMMAND(WIC64_CMD_ECHO, Echo),

        WIC64_COMMAND(WIC64_CMD_VERSION_STRING,  Version),
        WIC64_COMMAND(WIC64_CMD_VERSION_NUMBERS, Version),

        WIC64_COMMAND(WIC64_CMD_SCAN_WIFI_NETWORKS, Scan),
        WIC64_COMMAND(WIC64_CMD_CONNECT_WITH_SSID_STRING, Connect),
        WIC64_COMMAND(WIC64_CMD_CONNECT_WITH_SSID_INDEX,  Connect),

        WIC64_COMMAND(WIC64_CMD_GET_MAC,  MAC),
        WIC64_COMMAND(WIC64_CMD_GET_SSID, SSID),
        WIC64_COMMAND(WIC64_CMD_GET_RSSI, RSSI),
        WIC64_COMMAND(WIC64_CMD_GET_IP,   IP),

        WIC64_COMMAND(WIC64_CMD_HTTP_GET,         Http),
        WIC64_COMMAND(WIC64_CMD_HTTP_GET_ENCODED, Http),
        WIC64_COMMAND(WIC64_CMD_HTTP_POST,        Http),

        WIC64_COMMAND(WIC64_CMD_TCP_OPEN,  Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_READ,  Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_WRITE, Tcp),

        WIC64_COMMAND(WIC64_CMD_GET_SERVER, Server),
        WIC64_COMMAND(WIC64_CMD_SET_SERVER, Server),

        WIC64_COMMAND(WIC64_CMD_GET_TIMEZONE, Timezone),
        WIC64_COMMAND(WIC64_CMD_SET_TIMEZONE, Timezone),
        WIC64_COMMAND(WIC64_CMD_GET_LOCAL_TIME, Time),

        WIC64_COMMANDS_END
    };
}