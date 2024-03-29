
#include "tcp.h"
#include "commands.h"
#include "connection.h"
#include "tcpClient.h"
#include "utilities.h"

namespace WiC64 {
    const char* Tcp::TAG = "TCP";

    extern TcpClient* tcpClient;
    extern Connection* connection;

    const char* Tcp::describe() {
        switch (id()) {
            case WIC64_CMD_TCP_OPEN:
                return "TCP (open)";

            case WIC64_CMD_TCP_AVAILABLE:
                return "TCP (available)";

            case WIC64_CMD_TCP_READ:
                return "TCP (read)";

            case WIC64_CMD_TCP_WRITE:
                return "TCP (write)";
                break;

            case WIC64_CMD_TCP_CLOSE:
                return "TCP (close)";
                break;

            default: return "TCP (unknown)";
        }
    }

    void Tcp::execute(void) {
        char host[256];
        char portAsString[6];
        uint16_t port;
        int32_t size;

        if (!connection->ready()) {
            const char* message = !connection->connected()
                ? "WiFi not connected"
                : "No IP address assigned";

            ESP_LOGE(TAG, "Can't execute TCP command: %s", message);

            error(CONNECTION_ERROR, message, "!0");
            goto DONE;
        }

        if (id() == WIC64_CMD_TCP_OPEN) {
            if (request()->payload()->size() == 0) {
                error(CLIENT_ERROR, "No URL specified", "!0");
                goto DONE;
            }

            request()->payload()->field(0, ':', host);
            request()->payload()->field(1, ':', portAsString);
            port = atoi(portAsString);

            if (tcpClient->open(host, port)) {
                success("Success", "0");
            } else {
                error(NETWORK_ERROR, "Could not open connection", "!E");
            }
        }

        else if (id() == WIC64_CMD_TCP_CLOSE) {
            tcpClient->close();
            success("Success", "0");
        }

        else if (id() == WIC64_CMD_TCP_AVAILABLE) {
            if (tcpClient->connected()) {
                uint16_t available = tcpClient->available();
                response()->appendByte(LOWBYTE(available));
                response()->appendByte(HIGHBYTE(available));
            } else {
                error(NETWORK_ERROR, "TCP connection closed", "!E");
            }
        }

        else if (id() == WIC64_CMD_TCP_READ) {
            if (tcpClient->connected()) {
                if ((size = tcpClient->read(response()->data())) > -1) {
                    response()->size(size);
                }
            } else {
                error(NETWORK_ERROR, "TCP connection closed", "!E");
            }
        }

        else if (id() == WIC64_CMD_TCP_WRITE) {
            size = tcpClient->write(request()->payload());

            if (tcpClient->connected()) {
                if (size == request()->payload()->size()) {
                    success("Success", "0");
                } else {
                    error(NETWORK_ERROR, "Failed to write TCP data", "!E");
                }
            } else {
                error(NETWORK_ERROR, "TCP connection closed", "!E");
            }
        }
    DONE:
        responseReady();
    }
}
