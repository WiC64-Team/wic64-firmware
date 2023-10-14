#include <cstdlib>
#include <cstring>

#include "data.h"

namespace WiC64 {
    Data::Data() {
        m_data = transferBuffer;
     }

    Data::Data(uint8_t *data, uint32_t size) {
        this->set(data, size);
    }

    char* Data::c_str() {
        m_data[m_size] = '\0';
        return (char*) m_data;
    }

    void Data::set(uint8_t *data, uint32_t size) {
        m_data = data;
        m_size = size;
        m_data[m_size] = '\0';
    }

    void Data::set(const char *c_str) {
        m_data = (uint8_t*) c_str;
        m_size = strlen(c_str);
        m_data[m_size] = '\0';
    }

    void Data::set(Data *data) {
        set(data->data(), data->size());
    }

    // copies string content and adds a terminating nullbyte
    void Data::copyString(const char *c_str) {
        m_data = transferBuffer;
        m_size = strlen(c_str)+1;
        strncpy((char*) transferBuffer, c_str, m_size+1);
    }

    // copies string content without adding terminating nullbyte
    void Data::copyData(const char *c_str) {
        m_data = transferBuffer;
        m_size = strlen(c_str);
        strncpy((char*) transferBuffer, c_str, m_size+1);
    }

    void Data::appendField(const String& string) {
        size_t len = string.length();
        memcpy(m_data + m_index, string.c_str(), len);

        m_index += len;
        m_data[m_index] = '\1';
        m_index++;
        m_size += len + 1;
    }

    const char* Data::field(uint8_t index, char *dst) {
        return field(index, '\1', dst);
    }

    const char* Data::field(uint8_t index, char separator, char* dst) {
        dst[0] = '\0';

        char* begin = (char*) m_data;
        char* end = begin;
        char* previous_end = end;
        uint8_t size;

        for (uint16_t i=0; i<=index; i++) {
            previous_end = end;

            if ((end = strchr(end, separator)) != NULL) {
                if (index == i) {
                    begin = previous_end;

                    size =
                        (end - (char*) m_data) -
                        (begin - (char*) m_data);

                    memcpy(dst, begin, size);
                    dst[size] = '\0';
                    break;
                }
                end++;
            }
            else { // Last field, not terminated by separator byte
                begin = previous_end;
                size =  strlen(begin);

                if (size > 0) {
                    memcpy(dst, begin, size);
                    dst[size] = '\0';
                    break;
                }
            }
        }
        return dst;
    }

    void Data::size(uint32_t size) {
        m_size = size;
        m_data[size] = '\0';
    }

    void Data::queue(QueueHandle_t queue, uint32_t size) {
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

    int64_t Data::sizeToReport() {
        return (m_sizeToReport > -1) ? m_sizeToReport : m_size;
    }

    Data* Data::zeroTerminated() {
        if (m_data[m_size-1] != 0) {
            m_size++;
            m_data[m_size] = '\0';
        }
        return this;
    }
}