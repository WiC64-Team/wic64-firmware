#include "buttons.h"
#include "settings.h"
#include "userport.h"
#include "display.h"
#include "led.h"

namespace WiC64 {
    const char* Buttons::TAG = "BUTTONS";

    extern Settings *settings;
    extern Userport *userport;
    extern Display *display;
    extern Led *led;

    Buttons::Buttons() {

        // ESP-Button: short press => detach/attach to userport
        // This feature has been added in order to support ill-
        // designed userport expanders that do not properly
        // multiplex the userport but instead simply connect all
        // userport modules in parallel, causing conflicts if
        // more than one module is using the port at the same time

        m_esp_button.attachLongPressStop([] {
            if (userport->isConnected()) {
                userport->disconnect();
                display->userportConnected(false);
            } else {
                userport->connect();
                display->userportConnected(true);
            }
            settings->userportDisconnected(!userport->isConnected());
        });

        // Wic64-Button: short press => enable/disable LED
        m_wic64_button.attachClick([] {
            led->enabled(!led->enabled());
            display->notify(led->enabled()
                ? "LED ENABLED"
                : "LED DISABLED");
            settings->ledEnabled(led->enabled());
        });

        // WiC64-Button: long press => rotate display by 180°
        m_wic64_button.attachLongPressStop([] {
            display->rotated(!display->rotated());
            settings->displayRotated(display->rotated());
        });
    }

    void Buttons::tick(void) {
        m_esp_button.tick();
        m_wic64_button.tick();
    }
}
