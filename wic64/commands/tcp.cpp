
#include "tcp.h"
#include "commands.h"
#include "connection.h"
#include "tcpClient.h"

namespace WiC64 {
    const char* Tcp::TAG = "TCP";

    extern TcpClient* tcpClient;
    extern Connection* connection;

    const char* Tcp::describe() {
        switch (id()) {
            case WIC64_CMD_TCP_OPEN:
                return "TCP (open)";

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
                ? "WiFi not connected "
                : "No IP address assigned";

            ESP_LOGE(TAG, "Can't execute TCP command: %s", message);

            error(CONNECTION_ERROR, message, "!0");
            responseReady();
            return;
        }

        if (id() == WIC64_CMD_TCP_OPEN) {
            request()->payload()->field(0, ':', host);
            request()->payload()->field(1, ':', portAsString);
            port = atoi(portAsString);

            if (tcpClient->open(host, port)) {
                success("Success", "0");
            } else {
                error(NETWORK_ERROR, "Failed to open TCP connection", "!E");
            }
        }

        else if (id() == WIC64_CMD_TCP_CLOSE) {
            tcpClient->close();
            success("Success", "0");
        }

        else if (id() == WIC64_CMD_TCP_READ) {
            if ((size = tcpClient->read(response()->data())) > -1) {
                response()->size(size);
            }
        }

        else if (id() == WIC64_CMD_TCP_WRITE) {
            size = tcpClient->write(request()->payload()->zeroTerminated());

            if (size == request()->payload()->size()) {
                success("Success", "0");
            } else {
                error(NETWORK_ERROR, "Failed to write TCP data", "!E");
            }
        }
        responseReady();
    }
}
