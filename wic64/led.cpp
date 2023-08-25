#include "led.h"
#include "utilities.h"

namespace WiC64 {
    Led::Led(gpio_num_t gpio) {
        m_gpio = gpio;
        SET_LOW(m_gpio);
        enable();
    }

    void Led::enable(void) {
        gpio_set_direction(m_gpio, GPIO_MODE_OUTPUT);
    }

    void Led::disable(void) {
        gpio_set_direction(m_gpio, GPIO_MODE_INPUT);
    }

    void Led::on(void) {
        SET_HIGH(m_gpio);
    }

    void Led::off(void) {
        SET_LOW(m_gpio);
    }
}