#include "wic64.h"
#include "tcpClient.h"

namespace WiC64 {
    const char* TcpClient::TAG = "TCPCLIENT";

    TcpClient::TcpClient() {
        ESP_LOGI(TAG, "TCP client initialized");
    }

    int TcpClient::open(const char* host, const uint16_t port) {
        bool connected = false;
        if (m_client.connected()) {
            ESP_LOGW(TAG, "Closing previously opened connection");
            m_client.stop();
        }

        connected = m_client.connect(host, port, 3000);

        ESP_LOG_LEVEL((connected ? ESP_LOG_INFO : ESP_LOG_ERROR), TAG,
            "%s connection to %s on port %d",
            connected ? "Opened" : "Failed to open",
            host,
            port);

        return connected;
    }

    int64_t TcpClient::read(uint8_t* data) {
        int64_t read = -1;
        bool available = false;
        uint32_t started = millis();

        ESP_LOGI(TAG, "Waiting at most 500ms for data to become available");

        while ((millis() - started < 500)) {
            vTaskDelay(pdMS_TO_TICKS(10));
            if ((available = m_client.available())) {
                break;
            }
        }

        ESP_LOGI(TAG, "%s available after %ldms",
            available ? "Data" : "No data",
            available ? millis() - started : 500);

        if (available) {
            read = m_client.read(data, MAX_READ_CHUNK_SIZE);
            ESP_LOGI(TAG, "Read %lld bytes", read);
        }

        return read;
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
}