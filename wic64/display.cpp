#include "Wire.h"

#include "display.h"

namespace WiC64 {
    const char* Display::TAG = "DISPLAY";

    Display::Display() {
        ESP_LOGI(TAG, "Initializing I2C interface");
            if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
            ESP_LOGE(TAG, "Could not initialize I2C interface");
            return;
        }

        ESP_LOGI(TAG, "Creating display instance");
        display = new Adafruit_SSD1306(WIDTH, HEIGHT, &Wire, RESET_PIN_NOT_CONNECTED);

        ESP_LOGI(TAG, "Allocating SSD1306 buffer");
        if (!display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            ESP_LOGE(TAG, "Could not allocate buffer for SSD1306");
            display = NULL;
            return;
        }

        display->setTextColor(WHITE);
        display->setTextSize(1);

        display->clearDisplay();
        display->drawBitmap(0, 0, logo, WIDTH, HEIGHT, WHITE);
        display->display();
    }

    void Display::setRotation(uint8_t rotation) {
        this->rotation = rotation;
        update();
    }

    void Display::setIp(String ip) {
        this->ip = ip;
        update();
    }

    void Display::resetIp(void) {
        this->ip = "0.0.0.0";
        update();
    }

    void Display::setSSID(String ssid) {
        this->ssid = ssid;
        update();
    }

    void Display::setRSSI(int8_t rssi) {
        snprintf(this->rssi, MAX_CHARS_FOR_RSSI, "%ddBm", rssi);
        update();
    }

    void Display::setStatus(String status) {
        this->status = status;
        update();
    }

    char *Display::abbreviated(String string) {
        return abbreviated(string, MAX_CHARS_PER_LINE);
    }

    char *Display::abbreviated(String string, uint8_t width) {
        if (width > MAX_CHARS_PER_LINE) {
            width = MAX_CHARS_PER_LINE;
        }

        return strncpy(line, string.c_str(), width);
    }

    void Display::printStatusAndRSSI(void) {
        uint8_t rssi_width = strlen(rssi);
        display->print(abbreviated(status, MAX_CHARS_PER_LINE - rssi_width - 1));

        uint8_t abbreviated_status_width = strlen(line);
        uint8_t gap = MAX_CHARS_PER_LINE - abbreviated_status_width - rssi_width;

        while(gap--) display->print(" ");
        display->println(rssi);
    }

    void Display::update() {
        if (display == NULL) return;

        display->clearDisplay();
        display->setRotation(rotation);

        display->setFont(FONT_BIG);
        display->setCursor(0, 12);
        display->println(ip);

        display->setFont(FONT_BUILTIN);
        display->setCursor(0, 20);
        display->println(abbreviated(ssid));

        printStatusAndRSSI();

        display->display();
    }
}
