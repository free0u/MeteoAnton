#ifndef OLED_h
#define OLED_h

#include "MeteoLog.h"
#include "SSD1306.h"
#include "SensorsData.h"
#include "fonts.h"
#include "images.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

class OLED {
   private:
    SSD1306 *display;
    MeteoLog *meteoLog;

   public:
    bool alwaysOn;
    OLED() {}
    void init(MeteoLog *meteoLog) {
        this->meteoLog = meteoLog;
        // display = new SSD1306(0x3C, D3, D5);
        // display->init();
        // display->flipScreenVertically();
        // display->setContrast(1);
        // display->setFont(ArialMT_Plain_10);
        alwaysOn = false;
    }
    void displayIp(String uptime, String rtc, String ntp, int cachedCount) {
        // display->clear();

        // // display->fillRect(0, 0, 128, 64);
        // // display->display();
        // // return;

        // display->setFont(ArialMT_Plain_10);
        // display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);

        // String wifi = WiFi.SSID() + " (" + WiFi.localIP().toString() + ")\n";
        // String up = "up " + uptime + "\n";
        // String timeNtpRtc = "ntp " + ntp + " (" + rtc + ")\n";
        // String cached = "cached: " + String(cachedCount);
        // String text = wifi + timeNtpRtc + up + cached;
        // display->drawString(display->getWidth() / 2, display->getHeight() / 2, text);
        // // display->setContrast(contrast);
        // display->display();
    }

    void displayEmpty() {
        // display->clear();
        // display->display();
    }

    const int LEN = 18;
    long ts = -1000000;

    String firstInString(SensorsData &data) {
        // String res = "in    ";
        // float t = data.dsTempOne.getIfUpdated();
        // if (abs(t) < 1e-3) {
        //     res += " ";
        // } else if (t > 0) {
        //     res += "+";
        // }
        // char tempStr[5];
        // sprintf(tempStr, "%.1f", t);
        // res += tempStr;
        // res += "°";

        // if (millis() - ts > 2000) {
        //     ts = millis();
        //     Serial.println("#1 " + res);
        // }

        // return res;
    }

    void displaySensorsData(SensorsData *data) {
        // display->clear();

        // display->setTextAlignment(TEXT_ALIGN_LEFT);
        // display->setFont(Monospaced_plain_12);
        // // firstInString(data);
        // display->drawString(
        //     0, 0, "ds " + String(data->dsTempOne.getIfUpdated()) + " °C " + String(data->dsTempTwo.getIfUpdated()));
        // display->drawString(0, 16, "dht hum " + String(data->dhtHum.getIfUpdated()) + " %");
        // display->drawString(0, 32, "bme hum " + String(data->bmeHum.getIfUpdated()) + " %");
        // // display->drawString(0, 48, "pressure " + String(data.bmePressure.getIfUpdated()) + " mmHg");
        // // display->drawString(0, 48, "co2  " + String((int)data->co2.getIfUpdated()) + " ppm");

        // display->display();
    }

    void displayAutoOffSwitch() {
        // display->clear();
        // display->setFont(Monospaced_plain_12);
        // display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        // String status = alwaysOn ? "disabled" : "enabled";
        // display->drawString(display->getWidth() / 2, display->getHeight() / 2, "Auto off " + status);
        // // display->drawString(display->getWidth() / 2, display->getHeight() / 2 + 20, "Turn " + alwaysOn ? "off" :
        // // "on");
        // display->display();
    }

    void displayWifiChange() {
        // display->clear();
        // display->setFont(Monospaced_plain_12);
        // display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
        // display->drawString(display->getWidth() / 2, display->getHeight() / 2, "WiFi scan?");
        // display->display();
    }

    void displayLog() {
        // display->clear();

        // display->setTextAlignment(TEXT_ALIGN_LEFT);
        // display->setFont(ArialMT_Plain_10);

        // int messageCount = meteoLog->getCount();

        // for (int i = 0; i < messageCount; i++) {
        //     display->drawString(0, i * 10, "* " + meteoLog->get(i));
        // }

        // display->display();
    }
    void showMessage(String message) {
        meteoLog->add(message);
        displayLog();
    }
};

#endif
