#ifndef WIC64_DATA_H
#define WIC64_DATA_H

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "wic64.h"

namespace WiC64 {
    class Data {
        private:
            uint8_t *m_data = NULL;
            uint32_t m_index = 0;

            uint32_t m_size = 0;
            int64_t m_sizeToReport = -1;

            QueueHandle_t m_queue = NULL;
        public:
            Data();
            Data(uint8_t* data, uint32_t size);

            uint8_t* data() { return m_data; }
            char* c_str();

            void set(Data* data);
            void set(uint8_t* data, uint32_t size);
            void set(const char* c_str);
            void copyString(const char* c_str);
            void copyData(const char* c_str);

            void appendField(const String& string);
            const char* field(uint8_t index, char *dst);
            const char* field(uint8_t index, char separator, char *dst);

            uint32_t size() { return m_size; }
            void size(uint32_t size);

            void sizeToReport(int64_t sizeToReport) { m_sizeToReport = sizeToReport; }
            int64_t sizeToReport();

            Data* zeroTerminated();

            QueueHandle_t queue() { return m_queue; }
            void queue(QueueHandle_t queue, uint32_t size);

            bool isPresent(void);
            bool isEmpty(void);
            bool isQueued(void);
    };
}
#endif // WIC64_DATA_H