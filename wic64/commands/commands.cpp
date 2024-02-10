#include "commands.h"
#include "version.h"
#include "http.h"
#include "ip.h"
#include "server.h"
#include "scan.h"
#include "connect.h"
#include "configured.h"
#include "connected.h"
#include "ssid.h"
#include "rssi.h"
#include "mac.h"
#include "time.h"
#include "timezone.h"
#include "tcp.h"
#include "test.h"
#include "update.h"
#include "reboot.h"
#include "deprecated.h"
#include "undefined.h"
#include "status.h"
#include "timeout.h"
#include "hardware.h"

namespace WiC64 {
    WIC64_COMMANDS = {
        WIC64_COMMAND(WIC64_CMD_GET_VERSION_STRING,  Version),
        WIC64_COMMAND(WIC64_CMD_GET_VERSION_NUMBERS, Version),

        WIC64_COMMAND(WIC64_CMD_SCAN_WIFI_NETWORKS, Scan),
        WIC64_COMMAND(WIC64_CMD_CONNECT_WITH_SSID_STRING, Connect),
        WIC64_COMMAND(WIC64_CMD_CONNECT_WITH_SSID_INDEX,  Connect),
        WIC64_COMMAND(WIC64_CMD_IS_CONFIGURED, Configured),
        WIC64_COMMAND(WIC64_CMD_IS_CONNECTED, Connected),

        WIC64_COMMAND(WIC64_CMD_GET_MAC,  MAC),
        WIC64_COMMAND(WIC64_CMD_GET_SSID, SSID),
        WIC64_COMMAND(WIC64_CMD_GET_RSSI, RSSI),
        WIC64_COMMAND(WIC64_CMD_GET_IP,   IP),

        WIC64_COMMAND(WIC64_CMD_HTTP_GET,         Http),
        WIC64_COMMAND(WIC64_CMD_HTTP_GET_ENCODED, Http),
        WIC64_COMMAND(WIC64_CMD_HTTP_POST_URL,    Http),
        WIC64_COMMAND(WIC64_CMD_HTTP_POST_DATA,   Http),

        WIC64_COMMAND(WIC64_CMD_TCP_OPEN,       Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_AVAILABLE,  Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_READ,       Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_WRITE,      Tcp),
        WIC64_COMMAND(WIC64_CMD_TCP_CLOSE,      Tcp),

        WIC64_COMMAND(WIC64_CMD_GET_SERVER, Server),
        WIC64_COMMAND(WIC64_CMD_SET_SERVER, Server),

        WIC64_COMMAND(WIC64_CMD_GET_TIMEZONE, Timezone),
        WIC64_COMMAND(WIC64_CMD_SET_TIMEZONE, Timezone),
        WIC64_COMMAND(WIC64_CMD_GET_LOCAL_TIME, Time),

        WIC64_COMMAND(WIC64_CMD_UPDATE_FIRMWARE, Update),

        WIC64_COMMAND(WIC64_CMD_REBOOT, Reboot),
        WIC64_COMMAND(WIC64_CMD_GET_STATUS_MESSAGE, Status),
        WIC64_COMMAND(WIC64_CMD_SET_TRANSFER_TIMEOUT, Timeout),
        WIC64_COMMAND(WIC64_CMD_SET_REMOTE_TIMEOUT, Timeout),
        WIC64_COMMAND(WIC64_CMD_IS_HARDWARE, Hardware),

        WIC64_COMMAND(WIC64_CMD_FORCE_TIMEOUT, Test),
        WIC64_COMMAND(WIC64_CMD_FORCE_ERROR, Test),
        WIC64_COMMAND(WIC64_CMD_ECHO, Test),

        WIC64_COMMAND(WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_03,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_04,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_05,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_FIRMWARE_UPDATE_REQUIRED_18, Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_GET_STATS_07,                Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_LOG_TO_SERIAL_CONSOLE_09,    Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_GET_UPD_0A,                  Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_SEND_UPD_0B,                 Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_SET_UPD_PORT_0E,             Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_GET_UPD_DUPLICATE_1E,        Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_SEND_UPD_DUPLICATE_1F,       Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_GET_EXTERNAL_IP_13,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_GET_PREFERENCES_19,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_SET_PREFERENCES_1A,          Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_SET_TCP_PORT_20,             Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_LEGACY_HTTP_POST_24,         Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_BIG_LOADER_25,               Deprecated),
        WIC64_COMMAND(WIC64_CMD_DEPRECATED_FACTORY_RESET_63,            Deprecated),
        WIC64_COMMANDS_END
    };

    command_map_entry_t WIC64_COMMAND_UNDEFINED = WIC64_COMMAND(WIC64_CMD_NONE, Undefined);
}