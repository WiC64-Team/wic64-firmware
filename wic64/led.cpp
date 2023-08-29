#include "led.h"
#include "settings.h"
#include "utilities.h"

namespace WiC64 {
    extern Settings *settings;

    Led::Led() : Led(GPIO_NUM_2) { }

    Led::Led(gpio_num_t gpio) {
        m_gpio = gpio;
        SET_LOW(m_gpio);
        enabled(settings->ledEnabled());
    }

    bool Led::enabled(void) {
        return m_enabled;
    }

    void Led::enabled(bool enabled) {
        m_enabled = enabled;
        gpio_set_direction(m_gpio, m_enabled ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    }

    void Led::on(void) {
        SET_HIGH(m_gpio);
    }

    void Led::off(void) {
        SET_LOW(m_gpio);
    }
}