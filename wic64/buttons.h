#ifndef WIC64_BUTTONS_H
#define WIC64_BUTTONS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include "OneButton.h"

#pragma GCC diagnostic pop

namespace WiC64 {
    class Buttons {
        public: const static char* TAG;
        private:

        OneButton m_esp_button = OneButton(GPIO_NUM_0, true, true);
        OneButton m_wic64_button = OneButton(GPIO_NUM_33, false, false);

        uint32_t loop_ms;

        public:
            Buttons();
            void loop(void);
    };
}

#endif // WIC64_BUTTONS_H
