#ifndef WIC64_CLOCK_H
#define WIC64_CLOCK_H

#include "wic64.h"

namespace WiC64 {
    class Clock {
        public: static const char* TAG;

        private:
            void configure(void);
            void reconfigure(void);

        public:
            Clock();
            const char* localTime(void);
            int32_t timezone(void);
            void timezone(int32_t timezone);
    };
}

#endif // WIC64_CLOCK_H