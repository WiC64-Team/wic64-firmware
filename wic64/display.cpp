#include "Wire.h"

#include "display.h"
#include "settings.h"
#include "../generated-version.h"

namespace WiC64 {
    const char* Display::TAG = "DISPLAY";

    extern Display *display;
    extern Settings *settings;

    Display::Display() {
        m_rotated = settings->displayRotated();

        ESP_LOGD(TAG, "Initializing I2C interface");
            if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN)) {
            ESP_LOGE(TAG, "Could not initialize I2C interface");
            return;
        }

        ESP_LOGD(TAG, "Creating display instance");
        m_display = new Adafruit_SSD1306(WIDTH, HEIGHT, &Wire, RESET_PIN_NOT_CONNECTED);

        ESP_LOGD(TAG, "Allocating SSD1306 buffer");
        if (!m_display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            ESP_LOGE(TAG, "Could not allocate buffer for SSD1306");
            m_display = NULL;
            return;
        }

        ESP_LOGI(TAG, "Display initialized");

        m_display->setRotation(m_rotated ? 2 : 0);
        m_display->setTextColor(WHITE);
        m_display->setTextSize(1);

        m_display->clearDisplay();
        m_display->drawBitmap(0, 0, logo, WIDTH, HEIGHT, WHITE);
        m_display->display();
    }

    bool Display::rotated(void) {
        return m_rotated;
    }

    void Display::rotated(bool rotated) {
        m_rotated = rotated;
        update();
    }

    void Display::ip(const String& ip) {
        m_ip = ip;
        update();
    }

    void Display::SSID(const String&ssid) {
        m_ssid = ssid;
        update();
    }

    void Display::RSSI(int8_t rssi) {
        snprintf(m_rssi_buffer, MAX_CHARS_FOR_RSSI, "%ddBm", rssi);
        update();
    }

    void Display::status(const String& status) {
        m_status = status;
        update();
    }

    void Display::userportConnected(bool userportConnected) {
        m_userportConnected = userportConnected;
        update();
    }

    char *Display::abbreviated(const String& string) {
        return abbreviated(string, MAX_CHARS_PER_LINE);
    }

    char *Display::abbreviated(const String &string, uint8_t width) {
        if (width > MAX_CHARS_PER_LINE) {
            width = MAX_CHARS_PER_LINE;
        }

        return strncpy(m_line_buffer, string.c_str(), width);
    }

    void Display::printCenteredLine(const String &string) {
        int16_t x, y;
        uint16_t w, h;
        int16_t cx = m_display->getCursorX();
        int16_t cy = m_display->getCursorY();

        m_display->getTextBounds(string, cx, cy, &x, &y, &w, &h);
        int16_t margin = (m_display->width() - w) / 2;
        m_display->setCursor(cx + margin, cy);
        m_display->println(string);
    }

    void Display::printStatusAndRSSI(void) {
        uint8_t rssi_width = strlen(m_rssi_buffer);
        m_display->print(abbreviated(m_status, MAX_CHARS_PER_LINE - rssi_width - 1));

        uint8_t abbreviated_status_width = strlen(m_line_buffer);
        uint8_t gap = MAX_CHARS_PER_LINE - abbreviated_status_width - rssi_width;

        while(gap--) m_display->print(" ");
        m_display->println(m_rssi_buffer);
    }

    void Display::printFreeMemory(void) {
        uint8_t free_heap_size = esp_get_free_heap_size() / 1024;
        uint8_t minimum_free_heap_size = esp_get_minimum_free_heap_size() / 1024;
        snprintf(m_line_buffer, MAX_CHARS_PER_LINE, "%dkb free %dkb min",
            free_heap_size, minimum_free_heap_size);
        m_display->println(String(m_line_buffer));
    }

    void Display::update() {
        if (m_display == NULL) return;
        if (m_notifying) return;

        m_display->setRotation(m_rotated ? 2 : 0);
        m_display->clearDisplay();

        m_display->setFont(FONT_BIG);
        m_display->setCursor(0, 12);

        if (m_userportConnected) {
            m_display->println(m_ip);

            m_display->setFont(FONT_BUILTIN);
            m_display->setCursor(0, 18);
            m_display->println(abbreviated(m_ssid));

            printStatusAndRSSI();

            m_display->println(abbreviated("Firmware v" WIC64_VERSION_SHORT_STRING));
            printFreeMemory();
        }
        else {
            m_display->println("DEACTIVATED");

            m_display->setFont(FONT_BUILTIN);
            m_display->setCursor(0, 18);

            m_display->println();
            m_display->println("Hold ESP BOOT button");
            m_display->println("for one second to");
            m_display->println("activate WiC64");
        }

        m_display->display();
    }

    void Display::notify(const String &message) {
        m_notifying = true;
        m_display->setRotation(m_rotated ? 2 : 0);
        m_display->clearDisplay();

        m_display->setFont(FONT_BIG);
        m_display->setCursor(0, 12);

        m_display->println(message);

        m_display->display();

        xTaskCreatePinnedToCore(notificationTask, "NOTIFY", 4096, NULL, 5, NULL, 0);
    }

    void Display::notificationTask(void *) {
        vTaskDelay(1000);
        display->m_notifying = false;
        display->update();
        vTaskDelete(NULL);
    }
}
