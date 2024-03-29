#ifndef WIC64_COMMANDS_H
#define WIC64_COMMANDS_H

// ADD NEW COMMAND IDS HERE -------------------------------------------

#define WIC64_CMD_GET_VERSION_STRING  0x00
#define WIC64_CMD_GET_VERSION_NUMBERS 0x26

#define WIC64_CMD_SCAN_WIFI_NETWORKS       0x0c
#define WIC64_CMD_IS_CONFIGURED            0x2f
#define WIC64_CMD_IS_CONNECTED             0x2c
#define WIC64_CMD_CONNECT_WITH_SSID_STRING 0x02
#define WIC64_CMD_CONNECT_WITH_SSID_INDEX  0x0d

#define WIC64_CMD_GET_MAC  0x14
#define WIC64_CMD_GET_SSID 0x10
#define WIC64_CMD_GET_RSSI 0x11
#define WIC64_CMD_GET_IP   0x06

#define WIC64_CMD_HTTP_GET         0x01
#define WIC64_CMD_HTTP_GET_ENCODED 0x0f
#define WIC64_CMD_HTTP_POST_URL    0x28
#define WIC64_CMD_HTTP_POST_DATA   0x2b

#define WIC64_CMD_TCP_OPEN      0x21
#define WIC64_CMD_TCP_AVAILABLE 0x30
#define WIC64_CMD_TCP_READ      0x22
#define WIC64_CMD_TCP_WRITE     0x23
#define WIC64_CMD_TCP_CLOSE     0x2e

#define WIC64_CMD_GET_SERVER 0x12
#define WIC64_CMD_SET_SERVER 0x08

#define WIC64_CMD_GET_TIMEZONE   0x17
#define WIC64_CMD_SET_TIMEZONE   0x16
#define WIC64_CMD_GET_LOCAL_TIME 0x15

#define WIC64_CMD_UPDATE_FIRMWARE 0x27

#define WIC64_CMD_REBOOT 0x29
#define WIC64_CMD_GET_STATUS_MESSAGE 0x2a
#define WIC64_CMD_SET_TRANSFER_TIMEOUT 0x2d
#define WIC64_CMD_SET_REMOTE_TIMEOUT 0x32
#define WIC64_CMD_IS_HARDWARE 0x31

#define WIC64_CMD_FORCE_TIMEOUT 0xfc
#define WIC64_CMD_FORCE_ERROR 0xfd
#define WIC64_CMD_ECHO 0xfe

// Deprecated commands

#define WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_03 0x03
#define WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_04 0x04
#define WIC64_CMD_DEPRECATED_UPDATE_FIRMWARE_05 0x05
#define WIC64_CMD_DEPRECATED_FIRMWARE_UPDATE_REQUIRED_18 0x18
#define WIC64_CMD_DEPRECATED_GET_STATS_07 0x07
#define WIC64_CMD_DEPRECATED_LOG_TO_SERIAL_CONSOLE_09 0x09
#define WIC64_CMD_DEPRECATED_GET_UPD_0A 0x0a
#define WIC64_CMD_DEPRECATED_SEND_UPD_0B 0x0b
#define WIC64_CMD_DEPRECATED_SET_UPD_PORT_0E 0x0e
#define WIC64_CMD_DEPRECATED_GET_UPD_DUPLICATE_1E 0x1e
#define WIC64_CMD_DEPRECATED_SEND_UPD_DUPLICATE_1F 0x1f
#define WIC64_CMD_DEPRECATED_GET_EXTERNAL_IP_13 0x13
#define WIC64_CMD_DEPRECATED_GET_PREFERENCES_19 0x19
#define WIC64_CMD_DEPRECATED_SET_PREFERENCES_1A 0x1a
#define WIC64_CMD_DEPRECATED_SET_TCP_PORT_20 0x20
#define WIC64_CMD_DEPRECATED_BIG_LOADER_25 0x25
#define WIC64_CMD_DEPRECATED_FACTORY_RESET_63 0x63
#define WIC64_CMD_DEPRECATED_LEGACY_HTTP_POST_24 0x24

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
    extern command_map_entry_t WIC64_COMMAND_UNDEFINED;
}

#endif // WIC64_COMMANDS_H