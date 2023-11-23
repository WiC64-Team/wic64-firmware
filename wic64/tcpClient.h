#ifndef WIC64_TCP_CLIENT_H
#define WIC64_TCP_CLIENT_H

#include "data.h"

#include "WiFiClient.h"

namespace WiC64 {
    class TcpClient {
        public: static const char* TAG;

        private:
            WiFiClient m_client;
            static const uint16_t MAX_READ_CHUNK_SIZE = 8192;

        public:
            TcpClient();
            bool connected(void);
            int open(const char* host, const uint16_t port);
            int32_t available(void);
            int64_t read(uint8_t* data);
            int32_t write(Data* data);
            int32_t write(uint8_t* data, uint32_t size);
            void close(void);
    };
}

#endif // WIC64_TCP_CLIENT_H