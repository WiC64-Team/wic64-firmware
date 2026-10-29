#include "wic64.h"
#include "tcpClient.h"

#include "lwip/sockets.h"

namespace WiC64 {
    const char* TcpClient::TAG = "TCPCLIENT";

    TcpClient::TcpClient() {
        m_unconfirmed = (uint8_t*) malloc(MAX_READ_CHUNK_SIZE);
        ESP_LOGI(TAG, "TCP client initialized");
    }

    bool TcpClient::connected(void) {
        if (m_unconfirmed_size > 0) {
            return true;
        }

        if (!m_client.connected()) {
            return false;
        }

        // WiFiClient::connected() probes with a 0-byte recv(), which
        // cannot see the server closing the connection: peek at one
        // byte instead, once everything it sent has been read.
        if (m_client.available()) {
            return true;
        }

        uint8_t byte;
        int result = recv(m_client.fd(), &byte, 1, MSG_PEEK | MSG_DONTWAIT);

        if (result == 0 || (result < 0 && errno != EWOULDBLOCK && errno != EAGAIN)) {
            ESP_LOGI(TAG, "Connection closed by the server");
            return false;
        }
        return true;
    }

    int TcpClient::open(const char* host, const uint16_t port) {
        bool connected = false;
        m_unconfirmed_size = 0;

        if (m_client.connected()) {
            ESP_LOGW(TAG, "Closing previously opened connection");
            close();
        }

        connected = m_client.connect(host, port, 5000);

        ESP_LOG_LEVEL((connected ? ESP_LOG_INFO : ESP_LOG_ERROR), TAG,
            "%s connection to %s on port %d",
            connected ? "Opened" : "Failed to open",
            host,
            port);

        return connected;
    }

    int32_t TcpClient::available(void) {
        return m_unconfirmed_size + m_client.available();
    }

    int64_t TcpClient::read(uint8_t* data) {
        int64_t read = -1;

        if (m_unconfirmed_size > 0) {
            ESP_LOGW(TAG, "Sending the %d bytes of the last read again", m_unconfirmed_size);
            memcpy(data, m_unconfirmed, m_unconfirmed_size);
            return m_unconfirmed_size;
        }

        if (m_client.available()) {
            read = m_client.read(data, MAX_READ_CHUNK_SIZE);
            ESP_LOGI(TAG, "Read %lld bytes", read);

            if (read > 0) {
                memcpy(m_unconfirmed, data, read);
                m_unconfirmed_size = read;
            }
        }
        return read;
    }

    void TcpClient::confirmRead(void) {
        m_unconfirmed_size = 0;
    }

    int32_t TcpClient::write(Data *data) {
        return write(data->data(), data->size());
    }

    int32_t TcpClient::write(uint8_t *data, uint32_t size) {
        ESP_LOGI(TAG, "Writing %d bytes", size);

        int32_t written = m_client.write(data, size);

        if (written < size) {
            if (written <= 0) {
                ESP_LOGE(TAG, "Failed to write any data");
            }
            else {
                ESP_LOGE(TAG, "Wrote only %d of %d bytes", written, size);
            }
        }
        else {
            ESP_LOGI(TAG, "Wrote %d bytes", written);
        }

        return written;
    }

    void TcpClient::close(void) {
        ESP_LOGI(TAG, "Closing connection");
        m_unconfirmed_size = 0;
        m_client.stop();
    }
}