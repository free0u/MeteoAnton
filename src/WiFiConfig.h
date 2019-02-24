#ifndef WiFiConfig_h
#define WiFiConfig_h

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

class WiFiConfig {
  private:
    WiFiManager wifiManager;

  public:
    WiFiConfig() {}

    bool connect() {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.println("Connection Failed!");
            return false;
        }
        return true;
    }

    // void connectWiFi(bool needReset) {
    //     if (needReset) {
    //         wifiManager.resetSettings();
    //     }
    //     if (!wifiManager.autoConnect("NodeMCU-free0u")) {
    //         Serial.println("Failed to connect, we should reset as see if it connects");
    //         delay(3000);
    //         ESP.reset();
    //         delay(5000);
    //     }
    // }

    void startPortal() {
        wifiManager.resetSettings();
        wifiManager.setConfigPortalTimeout(240);
        wifiManager.startConfigPortal("NodeMCU-free0u");
    }
};

#endif
