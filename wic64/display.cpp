#include "Wire.h"

#include "display.h"

namespace WiC64 {
    const char* Display::TAG = "DISPLAY";

    Display::Display() {
        ESP_LOGD(TAG, "Initializing I2C interface");
            if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
            ESP_LOGE(TAG, "Could not initialize I2C interface");
            return;
        }

        ESP_LOGD(TAG, "Creating display instance");
        display = new Adafruit_SSD1306(WIDTH, HEIGHT, &Wire, RESET_PIN_NOT_CONNECTED);

        ESP_LOGD(TAG, "Allocating SSD1306 buffer");
        if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            ESP_LOGE(TAG, "Could not allocate buffer for SSD1306");
            display = NULL;
            return;
        }

        ESP_LOGI(TAG, "Display initialized");

        display->setTextColor(WHITE);
        display->setTextSize(1);

        display->clearDisplay();
        display->drawBitmap(0, 0, logo, WIDTH, HEIGHT, WHITE);
        display->display();
    }

    void Display::rotation(uint8_t rotation) {
        m_rotation = rotation;
        update();
    }

    void Display::ip(String ip) {
        m_ip = ip;
        update();
    }

    void Display::SSID(String ssid) {
        m_ssid = ssid;
        update();
    }

    void Display::RSSI(int8_t rssi) {
        snprintf(m_rssi_buffer, MAX_CHARS_FOR_RSSI, "%ddBm", rssi);
        update();
    }

    void Display::status(String status) {
        m_status = status;
        update();
    }

    char *Display::abbreviated(String string) {
        return abbreviated(string, MAX_CHARS_PER_LINE);
    }

    char *Display::abbreviated(String string, uint8_t width) {
        if (width > MAX_CHARS_PER_LINE) {
            width = MAX_CHARS_PER_LINE;
        }

        return strncpy(m_line_buffer, string.c_str(), width);
    }

    void Display::printStatusAndRSSI(void) {
        uint8_t rssi_width = strlen(m_rssi_buffer);
        display->print(abbreviated(m_status, MAX_CHARS_PER_LINE - rssi_width - 1));

        uint8_t abbreviated_status_width = strlen(m_line_buffer);
        uint8_t gap = MAX_CHARS_PER_LINE - abbreviated_status_width - rssi_width;

        while(gap--) display->print(" ");
        display->println(m_rssi_buffer);
    }

    void Display::update() {
        if (display == NULL) return;

        display->clearDisplay();
        display->setRotation(m_rotation);

        display->setFont(FONT_BIG);
        display->setCursor(0, 12);
        display->println(m_ip);

        display->setFont(FONT_BUILTIN);
        display->setCursor(0, 20);
        display->println(abbreviated(m_ssid));

        printStatusAndRSSI();

        display->display();
    }
}
