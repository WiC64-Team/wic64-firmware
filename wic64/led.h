#ifndef WIC64_LED_H
#define WIC64_LED_H

#include "wic64.h"
#include "driver/gpio.h"

namespace WiC64 {
    class Led {
        private:
            gpio_num_t m_gpio;
            bool m_enabled = true;

        public:
            Led();
            Led(gpio_num_t gpio);
            bool enabled(void);
            void enabled(bool enabled);
            void on(void);
            void off(void);
    };
}

#endif // WIC64_LED_H