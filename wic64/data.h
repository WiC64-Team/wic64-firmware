#ifndef WIC64_DATA_H
#define WIC64_DATA_H

#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace WiC64 {
    class Data {
        private:
            bool m_allocated = false;

            uint8_t *m_data = NULL;
            QueueHandle_t m_queue = NULL;

            uint16_t m_size = 0;
            int32_t m_sizeToReport = -1;

            void freeIfAllocated(void);

        public:
            Data();
            Data(uint16_t size);
            Data(uint8_t* data, uint16_t size);
            Data(const char* c_str);
            ~Data();

            uint8_t* data() { return m_data; }
            void data(uint8_t* data, uint16_t size);
            void data(const char* c_str);

            uint16_t size() { return m_size; }

            void sizeToReport(uint16_t sizeToReport) { m_sizeToReport = sizeToReport; }
            uint16_t sizeToReport();

            QueueHandle_t queue() { return m_queue; }
            void queue(QueueHandle_t queue, uint16_t size);

            bool isPresent(void);
            bool isEmpty(void);
            bool isQueued(void);
    };
}
#endif // WIC64_DATA_H