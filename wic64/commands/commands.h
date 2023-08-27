#ifndef WIC64_COMMANDS_H
#define WIC64_COMMANDS_H

// ADD NEW COMMAND IDS HERE -------------------------------------------

#define WIC64_CMD_ECHO 0xfe

#define WIC64_CMD_VERSION_STRING  0x00
#define WIC64_CMD_VERSION_NUMBERS 0x26

#define WIC64_CMD_SCAN_WIFI_NETWORKS       0x0c
#define WIC64_CMD_CONNECT_WITH_SSID_STRING 0x02
#define WIC64_CMD_CONNECT_WITH_SSID_INDEX  0x0d

#define WIC64_CMD_GET_MAC  0x14
#define WIC64_CMD_GET_SSID 0x10
#define WIC64_CMD_GET_RSSI 0x11
#define WIC64_CMD_GET_IP   0x06

#define WIC64_CMD_HTTP_GET         0x01
#define WIC64_CMD_HTTP_GET_ENCODED 0x0f
#define WIC64_CMD_HTTP_POST        0x24

#define WIC64_CMD_TCP_OPEN  0x21
#define WIC64_CMD_TCP_READ  0x22
#define WIC64_CMD_TCP_WRITE 0x23

#define WIC64_CMD_GET_SERVER 0x12
#define WIC64_CMD_SET_SERVER 0x08

#define WIC64_CMD_GET_TIMEZONE   0x17
#define WIC64_CMD_SET_TIMEZONE   0x16
#define WIC64_CMD_GET_LOCAL_TIME 0x15

// DO NOT EDIT BEYOND THIS LINE ---------------------------------------------

#define WIC64_CMD_NONE 0xff
#define WIC64_COMMANDS command_map_entry_t commands[]
#define WIC64_COMMAND(ID, CLASS) { .id = ID, .create = [](Request* request) { return new CLASS(request); } }
#define WIC64_COMMANDS_END WIC64_COMMAND(WIC64_CMD_NONE, Command)

#include <functional>

namespace WiC64 {
    class Command;
    class Request;

    struct command_map_entry_t {
        uint8_t id;
        std::function<Command* (Request *request)> create;
    };

    extern WIC64_COMMANDS;
}

#endif // WIC64_COMMANDS_H