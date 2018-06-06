#ifndef OLED_h
#define OLED_h

#include "SSD1306.h"
#include "images.h"
#include "SensorsData.h"
#include "fonts.h"
#include "MeteoLog.h"

class OLED {
  private:
    SSD1306 *display;
    MeteoLog *meteoLog;
    bool printToSerial = true;

  public:
    OLED(MeteoLog *meteoLog) {
        this->meteoLog = meteoLog;
        display = new SSD1306(0x3C, D3, D5);
        display->init();
        display->flipScreenVertically();
        display->setContrast(255);
        display->setFont(ArialMT_Plain_10);
    }
    void displayIp(int cnt, String time) {
        display->clear();
        display->setFont(ArialMT_Plain_10);
        display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        display->drawString(display->getWidth() / 2, display->getHeight() / 2,
                            "IP:\n" + WiFi.localIP().toString() + "\n# " + String(cnt) + "\n" + time);
        display->display();
    }
    void displaySensorsData(SensorsData data) {
        display->clear();

        display->setTextAlignment(TEXT_ALIGN_LEFT);
        display->setFont(Monospaced_plain_12);
        display->drawString(0, 0, "dsTemp " + String(data.dsTemp) + " °C");
        display->drawString(0, 16, "dht hum " + String(data.dhtHum) + " %");
        display->drawString(0, 32, "bme hum " + String(data.bmeHum) + " %");
        display->drawString(0, 48, "pressure " + String(data.bmePressure) + " mmHg");

        display->display();
    }
    void log() {
        display->clear();

        display->setTextAlignment(TEXT_ALIGN_LEFT);
        display->setFont(ArialMT_Plain_10);

        int messageCount = meteoLog->getCount();

        for (int i = 0; i < messageCount; i++) {
            display->drawString(0, i * 10, "* " + meteoLog->get(i));
        }

        display->display();
    }
    void setPrintToSerial(bool f) { printToSerial = f; }
    void showMessage(String message) {
        meteoLog->add(message);
        if (printToSerial) {
            Serial.println("* " + message);
        }
        log();
    }
};

#endif
