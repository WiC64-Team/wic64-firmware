#include <cstdlib>
#include <cstring>

#include "data.h"

namespace WiC64 {
    Data::Data() { }

    Data::Data(uint8_t *data, uint16_t size) {
        this->data(data, size);
    }

    Data::Data(const char* c_str) {
        this->data(c_str);
    }

    char* Data::c_str() {
        m_data[m_size] = '\0';
        return (char*) m_data;
    }

    void Data::data(uint8_t *data, uint16_t size) {
        m_data = data;
        m_size = size;
    }

    void Data::data(const char *c_str) {
        m_data = (uint8_t*) c_str;
        m_size = strlen(c_str);
        m_data[m_size] = '\0';
        m_size++;
    }

    void Data::queue(QueueHandle_t queue, uint16_t size) {
        m_queue = queue;
        m_size = size;
    }

    bool Data::isPresent(void)
    {
        return m_size > 0;
    }

    bool Data::isEmpty(void) {
        return !isPresent();
    }

    bool Data::isQueued(void) {
        return m_queue != NULL;
    }

    uint16_t Data::sizeToReport() {
        return (m_sizeToReport > -1) ? m_sizeToReport : m_size;
    }
}