#ifndef WIC64_DATA_H
#define WIC64_DATA_H

#include <cstdint>

namespace WiC64 {
    class Data {
        private:
            uint8_t *m_data;
            uint16_t m_size;

        public:
            Data(uint16_t size);
            ~Data();

            bool isPresent(void);
            bool isEmpty(void);

            uint8_t* data() { return m_data; }
            uint16_t size() { return m_size; }
    };
}
#endif // WIC64_DATA_H