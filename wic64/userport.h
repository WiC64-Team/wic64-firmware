#ifndef WIC64_USERPORT_H
#define WIC64_USERPORT_H

#include <cstdint>

#include "driver/gpio.h"

class Userport {
    private:
        // Port: CIA2 Pin name -> ESP32 pin number // userport connector pin Name
        const gpio_num_t PB0 = GPIO_NUM_16; // C
        const gpio_num_t PB1 = GPIO_NUM_17; // D
        const gpio_num_t PB2 = GPIO_NUM_18; // E
        const gpio_num_t PB3 = GPIO_NUM_19; // F
        const gpio_num_t PB4 = GPIO_NUM_21; // H
        const gpio_num_t PB5 = GPIO_NUM_22; // J
        const gpio_num_t PB6 = GPIO_NUM_23; // K
        const gpio_num_t PB7 = GPIO_NUM_25; // L

        const uint64_t PORT_MASK = 1ULL<<PB0 | 1ULL<<PB1 | 1ULL<<PB2 | 1ULL<<PB3 | 1ULL<<PB4 | 1ULL<<PB5 | 1ULL<<PB6 | 1ULL<<PB7;
        const gpio_num_t PORT_PIN[8] = { PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7 };

        // Control signals
        const gpio_num_t PC2   = GPIO_NUM_14; // PC2   = 8  - signal from C64 to ESP - C64 triggers PC2 IRQ when CIA data register has been read or written to
        const gpio_num_t PA2   = GPIO_NUM_27; // PA2   = M  - signal from C64 to ESP - LOW = ESP may send data to C64 / HIGH = C64 is currently sending data to ESP (C64 powered on = HIGH)
        const gpio_num_t FLAG2 = GPIO_NUM_26; // FLAG2 = B  - signal from ESP to C64 - Switches quickly from HIGH to LOW when a byte is pending on the bus for C64 - Triggers IRQ on C64 (Byte is ready for pick-up)

        gpio_config_t port_config = {
            .pin_bit_mask = (1ULL<<PB0 | 1ULL<<PB1 | 1ULL<<PB2 | 1ULL<<PB3 | 1ULL<<PB4 | 1ULL<<PB5 | 1ULL<<PB6 | 1ULL<<PB7),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        gpio_config_t pc2_and_pa2_config = {
            .pin_bit_mask = (1ULL<<PC2 | 1ULL<<PA2),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        gpio_config_t flag2_config = {
            .pin_bit_mask = (1ULL<<FLAG2),
            .mode = GPIO_MODE_INPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        bool connected = false;

        uint16_t timeout;
        void (* timeoutHandler)();

        uint8_t *buffer;
        uint16_t size;
        uint16_t pos;

        void input(void);
        void output(void);

        void read(void);
        void write(void);
        void handshake();

    public:
        Userport(void);

        void setTimeout(uint16_t ms);
        void onTimeout(void (* onTimeout)());

        bool isConnected(void);
        void connect(void);
        void disconnect(void);

        bool isReady(void);

        void send(uint8_t *data, uint16_t size, void (* onSuccess)());
        void receive(uint8_t *data, uint16_t size, void (* onSuccess)(uint8_t* data, uint16_t size));
};
#endif // WIC64_USERPORT_H