#ifndef WIC64_REQUEST_H
#define WIC64_REQUEST_H

#include <cstdint>
#include "data.h"

namespace WiC64 {
    class Request {
        private:
            uint8_t m_id;
            uint8_t m_argc;
            Data** m_argv;

            int16_t getNextFreeArgumentIndex();

        public:
            Request(uint8_t api, uint8_t id, uint8_t argc);
            ~Request();

            uint8_t id(void) { return m_id; };
            uint8_t argc(void) { return m_argc; };

            bool hasArguments(void);
            Data* addArgument(Data* argument);
            Data* argument(void);
            Data* argument(uint8_t index);
    };
}

#endif // WIC64_REQUEST_H