#ifndef WIC64_USERPORT_H
#define WIC64_USERPORT_H

#include <cstdint>
#include "driver/gpio.h"

#define IS_HIGH(PIN) (((GPIO.in >> PIN) & 1) == 1)
#define IS_LOW(PIN) (((GPIO.in >> PIN) & 1) == 1)

#define SET_HIGH(PIN) (GPIO.out_w1ts = (1ULL<<PIN))
#define SET_LOW(PIN) (GPIO.out_w1tc = (1ULL<<PIN))

class Userport {
    private:
        // Port Register: CIA2 Pin name -> ESP32 pin number
        const gpio_num_t PB0 = GPIO_NUM_16;
        const gpio_num_t PB1 = GPIO_NUM_17;
        const gpio_num_t PB2 = GPIO_NUM_18;
        const gpio_num_t PB3 = GPIO_NUM_19;
        const gpio_num_t PB4 = GPIO_NUM_21;
        const gpio_num_t PB5 = GPIO_NUM_22;
        const gpio_num_t PB6 = GPIO_NUM_23;
        const gpio_num_t PB7 = GPIO_NUM_25;

        const uint64_t PORT_MASK =  1ULL<<PB0 | 1ULL<<PB1 | 1ULL<<PB2 | 1ULL<<PB3 |
                                    1ULL<<PB4 | 1ULL<<PB5 | 1ULL<<PB6 | 1ULL<<PB7;

        const gpio_num_t PORT_PIN[8] = { PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7 };

        /* Control signals
         *
         * PC2   : Handshake: C64 => ESP (ack/strobe: byte read from/written to port) (rising edge)
         * FLAG2 : Handshake: ESP => C64 (ack/strobe: byte read from/written to port) (falling edge)
         * PA2   : Direction: HIGH = C64 => ESP, LOW = ESP => C64
         */

        const gpio_num_t PC2   = GPIO_NUM_14;
        const gpio_num_t PA2   = GPIO_NUM_27;
        const gpio_num_t FLAG2 = GPIO_NUM_26;

        // synomyms for readability
        const gpio_num_t HANDSHAKE_LINE_C64_TO_ESP = PC2;
        const gpio_num_t HANDSHAKE_LINE_ESP_TO_C64 = FLAG2;
        const gpio_num_t DATA_DIRECTION_LINE = PA2;

        gpio_config_t port_config = {
            .pin_bit_mask = PORT_MASK,
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
        bool isReadyToSend(void);
        bool isReadyToReceive(void);

        void connect(void);
        void disconnect(void);

        void send(uint8_t *data, uint16_t size, void (* onSuccess)());
        void receive(uint8_t *data, uint16_t size, void (* onSuccess)(uint8_t* data, uint16_t size));
};
#endif // WIC64_USERPORT_H