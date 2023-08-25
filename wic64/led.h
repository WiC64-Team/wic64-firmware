#ifndef WIC64_LED_H
#define WIC64_LED_H

#include "wic64.h"
#include "driver/gpio.h"

namespace WiC64 {
    class Led {
        private:
            gpio_num_t m_gpio;

        public:
            Led(gpio_num_t gpio);
            void enable(void);
            void disable(void);
            void on(void);
            void off(void);
    };
}

#endif // WIC64_LED_H