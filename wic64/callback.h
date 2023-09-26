#ifndef WIC64_CALLBACK_H
#define WIC64_CALLBACK_H

#include <cstdint>

namespace WiC64 {
    typedef void (*callback_t) (uint8_t* data, uint16_t size);
}

#endif //WIC64_CALLBACK_H