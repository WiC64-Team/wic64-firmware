#ifndef WIC64_USERPORT_H
#define WIC64_USERPORT_H

#include <cstdint>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"

#define IS_HIGH(PIN) (((GPIO.in >> PIN) & 1) == 1)
#define IS_LOW(PIN) (((GPIO.in >> PIN) & 1) == 1)

#define SET_HIGH(PIN) (GPIO.out_w1ts = (1ULL<<PIN))
#define SET_LOW(PIN) (GPIO.out_w1tc = (1ULL<<PIN))

#ifdef ___cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(USERPORT_EVENTS);

enum {
    USERPORT_DATA_DIRECTION_CHANGED,
    USERPORT_HANDSHAKE_SIGNAL_RECEIVED,
};

#ifdef ___cplusplus
}
#endif

namespace WiC64 {

typedef void (*callback_t) (uint8_t* data, uint16_t size);
    class Userport {
        public: static const char* TAG;

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

            const uint64_t PORT_MASK = 1ULL<<PB0 | 1ULL<<PB1 | 1ULL<<PB2 | 1ULL<<PB3 |
                                    1ULL<<PB4 | 1ULL<<PB5 | 1ULL<<PB6 | 1ULL<<PB7;

            const gpio_num_t PORT_PIN[8] = { PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7 };

            /* Control signals
            *
            * PC2   : Handshake: C64 => ESP (ack/strobe: byte read from/written to port) (rising edge)
            * FLAG2 : Handshake: ESP => C64 (ack/strobe: byte read from/written to port) (falling edge)
            * PA2   : Direction: HIGH = C64 => ESP, LOW = ESP => C64
            */

            static const gpio_num_t PC2   = GPIO_NUM_14;
            static const gpio_num_t PA2   = GPIO_NUM_27;
            static const gpio_num_t FLAG2 = GPIO_NUM_26;

            // synomyms for readability
            static const gpio_num_t HANDSHAKE_LINE_C64_TO_ESP = PC2;
            static const gpio_num_t HANDSHAKE_LINE_ESP_TO_C64 = FLAG2;
            static const gpio_num_t DATA_DIRECTION_LINE = PA2;

            gpio_config_t port_config = {
                .pin_bit_mask = PORT_MASK,
                .mode = GPIO_MODE_INPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE,
            };

            enum TRANSFER_TYPE {
                TRANSFER_TYPE_NONE,
                TRANSFER_TYPE_SEND_FULL,
                TRANSFER_TYPE_SEND_PARTIAL,
                TRANSFER_TYPE_RECEIVE_FULL,
                TRANSFER_TYPE_RECEIVE_PARTIAL,
            };

            enum TRANSFER_STATE {
                TRANSFER_STATE_NONE,
                TRANSFER_STATE_PENDING,
                TRANSFER_STATE_RUNNING,
            };

            bool connected = false;

            static const uint16_t TIMEOUT_DEFAULT_1000MS = 1000;
            uint16_t timeout = TIMEOUT_DEFAULT_1000MS;
            uint32_t timeOfLastActivity;

            TaskHandle_t timeoutTaskHandle;
            bool timeoutTaskCreated = false;

            TRANSFER_TYPE transferType = TRANSFER_TYPE_NONE;
            TRANSFER_TYPE previousTransferType = TRANSFER_TYPE_NONE;
            TRANSFER_STATE transferState = TRANSFER_STATE_NONE;

            uint8_t *buffer;
            uint16_t size;
            uint16_t pos;

            callback_t onSuccessCallback = NULL;
            callback_t onFailureCallback = NULL;

            void setPortToInput(void);
            void setPortToOutput(void);

            inline void readByte(uint8_t *byte);
            inline void readNextByte(void);

            inline void writeByte(uint8_t *byte);
            inline void writeNextByte(void);

            inline void sendHandshakeSignal();

            void startTransfer(
                TRANSFER_TYPE type,
                uint8_t *data,
                uint16_t size,
                callback_t onSuccess,
                callback_t onFailure
            );
            inline void continueTransfer(void);
            void completeTransfer(void);
            void abortTransfer(const char* reason);

            void createTimeoutTask(void);
            void deleteTimeoutTask(void);
            bool isTimeoutTaskRunning(void);

        public:
            esp_event_loop_handle_t event_loop_handle;

            Userport();

            void connect(void);
            void disconnect(void);
            bool isConnected(void);
            bool isReadyToReceiveRequest(void);

            bool isTransferPending(void);
            void setTransferRunning(void);
            bool isSending(void);
            bool isSending(TRANSFER_TYPE type);

            void acceptRequest(void);

            void receivePartial(uint8_t *data, uint16_t size, callback_t onSuccess);
            void receivePartial(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure);
            void receive(uint8_t *data, uint16_t size, callback_t onSuccess);
            void receive(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure);

            void sendPartial(uint8_t *data, uint16_t size, callback_t onSuccess);
            void sendPartial(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure);
            void send(uint8_t *data, uint16_t size, callback_t onSuccess);
            void send(uint8_t *data, uint16_t size, callback_t onSuccess, callback_t onFailure);

            void resetTimeout(void);
            bool hasTimedOut(void);

            static void timeoutTask(void*);

            static void IRAM_ATTR dataDirectionChangedISR(void);
            static void onDataDirectionChanged(void* arg, esp_event_base_t base, int32_t id, void* data);

            static void IRAM_ATTR handshakeSignalReceivedISR(void);
            static void onHandshakeSignalReceived(void* arg, esp_event_base_t base, int32_t id, void* data);
    };

}
#endif // WIC64_USERPORT_H