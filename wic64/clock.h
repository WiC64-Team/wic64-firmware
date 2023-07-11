#ifndef WIC64_CLOCK_H
#define WIC64_CLOCK_H

#include "wic64.h"

namespace WiC64 {
    class Clock {
        public: static const char* TAG;

        private:
            static const int32_t timezones[];
            void configure(void);
            void reconfigure(void);

        public:
            Clock();
            const char* localTime(void);
    };
}

#endif // WIC64_CLOCK_H