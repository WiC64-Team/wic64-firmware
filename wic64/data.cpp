#include <cstdlib>
#include <cstring>

#include "data.h"

namespace WiC64 {
    Data::Data() { }

    Data::Data(uint16_t size) : m_size(size) {
        m_data = (uint8_t*) calloc(m_size > 0 ? size : 1, sizeof(uint8_t) + 1);
        m_allocated = m_data != NULL;
    }

    Data::Data(uint8_t *data, uint16_t size) {
        this->data(data, size);
    }

    Data::Data(const char* c_str) {
        this->data(c_str);
    }

    Data::~Data() {
        freeIfAllocated();
    }

    void Data::freeIfAllocated(void) {
        if (m_allocated && m_data != NULL) {
            free(m_data);
        }
    }

    void Data::data(uint8_t *data, uint16_t size) {
        freeIfAllocated();
        m_data = data;
        m_size = size;
        m_allocated = false;
    }

    void Data::data(const char *c_str) {
        freeIfAllocated();
        m_data = (uint8_t*) c_str;
        m_size = strlen(c_str)+1;
        m_allocated = false;
    }

    void Data::queue(QueueHandle_t queue, uint16_t size) {
        freeIfAllocated();
        m_queue = queue;
        m_size = size;
        m_allocated = false;
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