#include <cstdint>
#include <cstdlib>
#include "esp_log.h"

#include "request.h"
#include "data.h"

namespace WiC64 {
    const char* Request::TAG = "REQUEST";

    Request::Request(uint8_t api, uint8_t id, uint8_t argc) : m_api(api), m_id(id), m_argc(argc) {
        m_argv = (Data**) calloc(m_argc, sizeof(Data*));
    }

    Request::~Request() {
        for (uint8_t i=0; i<m_argc; i++) {
            if (m_argv[i] != NULL) {
                delete m_argv[i];
            }
        }

        if(m_argv != NULL) {
            free(m_argv);
        }
    }

    int16_t Request::getNextFreeArgumentIndex() {
        for (uint8_t i=0; i<m_argc; i++) {
            if (m_argv[i] == NULL) {
                return i;
            }
        }
        return -1;
    }

    bool Request::hasArguments(void) {
        return m_argc > 0;
    }

    Data *Request::addArgument(Data *argument)
    {
        int16_t index = getNextFreeArgumentIndex();

        if (index == -1) {
            ESP_LOGE(TAG, "All %d arguments have already been added", m_argc);
            return NULL;
        }

        m_argv[(uint8_t)index] = argument;
        return argument;
    }

    Data* Request::argument() {
        return m_argv[0];
    }

    Data* Request::argument(uint8_t index) {
        return m_argv[index];
    }
}