#ifndef WiFiConfig_h
#define WiFiConfig_h

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

class WiFiConfig {
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
