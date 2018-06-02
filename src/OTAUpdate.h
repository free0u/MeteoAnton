#ifndef OTAUpdate_h
#define OTAUpdate_h

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class OTAUpdate {
private:
    WiFiManager wifiManager;
public:
    void connectWiFi(bool needReset) {
        if (needReset) {
            wifiManager.resetSettings();
        }
        if (!wifiManager.autoConnect("NodeMCU-free0u")) {
            Serial.println("Failed to connect, we should reset as see if it connects");
            delay(3000);
            ESP.reset();
            delay(5000);
        }
    }
};

#endif
