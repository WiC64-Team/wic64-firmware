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
        m_data = data;
        m_size = size;
        m_allocated = false;
    }

    Data::Data(const char* c_str) {
        m_data = (uint8_t*) c_str;
        m_size = strlen(c_str)+1;
        m_allocated = false;
    }

    Data::~Data() {
        if (m_allocated) {
            free(m_data);
        }
    }

    bool Data::isPresent(void) {
        return m_size > 0;
    }

    bool Data::isEmpty(void) {
        return !isPresent();
    }
}