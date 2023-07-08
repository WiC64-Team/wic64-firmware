#include <cstdlib>
#include <cstring>

#include "data.h"

namespace WiC64 {
    Data::Data() {
        m_data = transferBuffer;
     }

    Data::Data(uint8_t *data, uint16_t size) {
        this->wrap(data, size);
    }

    Data::Data(const char* c_str) {
        this->wrap(c_str);
    }

    char* Data::c_str() {
        m_data[m_size] = '\0';
        return (char*) m_data;
    }

    void Data::wrap(uint8_t *data, uint16_t size) {
        m_data = data;
        m_size = size;
    }

    void Data::wrap(const char *c_str) {
        m_data = (uint8_t*) c_str;
        m_size = strlen(c_str);
        m_data[m_size] = '\0';
    }

    void Data::copyFrom(const char *c_str) {
        m_data = transferBuffer;
        m_size = strlen(c_str);
        strncpy((char*) transferBuffer, c_str, m_size+1);
    }

    void Data::appendSeparated(const String& string, const char separator) {
        size_t len = string.length();
        memcpy(m_data + m_index, string.c_str(), len);

        m_index += len;
        m_data[m_index] = separator;
        m_index++;
        m_size += len + 1;
    }

    void Data::appendSeparated(const String &string) {
        appendSeparated(string, '\1');
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