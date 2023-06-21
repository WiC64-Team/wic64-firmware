#include <cstdlib>
#include "data.h"

namespace WiC64 {
    Data::Data(uint16_t size) : m_size(size) {
        m_data = (uint8_t*) calloc(m_size, sizeof(uint8_t));
    }

    Data::~Data() {
        if(m_data != NULL) {
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