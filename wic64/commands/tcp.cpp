
#include "tcp.h"
#include "tcpClient.h"

namespace WiC64 {
    const char* Tcp::TAG = "TCP";
    extern TcpClient* tcpClient;

    const char* Tcp::describe() {
        switch (request()->id()) {
            case 0x21: return "TCP (open)"; break;
            case 0x22: return "TCP (read)"; break;
            case 0x23: return "TCP (write)"; break;
            default: return "TCP (unknown)"; break;
        }
    }

    void Tcp::execute(void) {
        char host[256];
        char portAsString[6];
        uint16_t port;
        int32_t size;

        if (request()->id() == 0x21) {
            request()->argument()->field(0, ':', host);
            request()->argument()->field(1, ':', portAsString);
            port = atoi(portAsString);

            response()->copy(tcpClient->open(host, port) ? "0" : "!E");
        }

        else if (request()->id() == 0x22) {
            if ((size = tcpClient->read(response()->data())) > -1) {
                response()->size(size);
            }
        }

        else if (request()->id() == 0x23) {
            size = tcpClient->write(request()->argument()->zeroTerminated());
            response()->copy(size == request()->argument()->size() ? "0" : "!E");
        }
        responseReady();
    }
}
