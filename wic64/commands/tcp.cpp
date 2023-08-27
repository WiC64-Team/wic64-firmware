
#include "tcp.h"
#include "commands.h"
#include "tcpClient.h"

namespace WiC64 {
    const char* Tcp::TAG = "TCP";
    extern TcpClient* tcpClient;

    const char* Tcp::describe() {
        switch (id()) {
            case WIC64_CMD_TCP_OPEN:
                return "TCP (open)";

            case WIC64_CMD_TCP_READ:
                return "TCP (read)";

            case WIC64_CMD_TCP_WRITE:
                return "TCP (write)";
                break;

            default: return "TCP (unknown)";
        }
    }

    void Tcp::execute(void) {
        char host[256];
        char portAsString[6];
        uint16_t port;
        int32_t size;

        if (id() == WIC64_CMD_TCP_OPEN) {
            request()->argument()->field(0, ':', host);
            request()->argument()->field(1, ':', portAsString);
            port = atoi(portAsString);

            response()->copy(tcpClient->open(host, port) ? "0" : "!E");
        }

        else if (id() == WIC64_CMD_TCP_READ) {
            if ((size = tcpClient->read(response()->data())) > -1) {
                response()->size(size);
            }
        }

        else if (id() == WIC64_CMD_TCP_WRITE) {
            size = tcpClient->write(request()->argument()->zeroTerminated());
            response()->copy(size == request()->argument()->size() ? "0" : "!E");
        }
        responseReady();
    }
}
