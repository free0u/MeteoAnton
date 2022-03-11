#ifndef WiFiConfig_h
#define WiFiConfig_h

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>

#include "modules/led/Led.h"
#include "system/MeteoLog.h"

struct WiFiCred {
    String ssid;
    String password;

    WiFiCred(String _ssid, String _pass) {
        ssid = _ssid;
        password = _pass;
    }
};

typedef std::vector<WiFiCred> WiFiCredList;

class WiFiConfig {
   private:
    bool isInited = false;
    WiFiManager* wifiManager;
    bool wifiManagerIsInited = false;
    WiFiCredList wifiList;
    MeteoLog* log;
    Led* led;

    void insertCredsIntoWm(ESP8266WiFiMulti& wm) {
        for (unsigned int i = 0; i < wifiList.size(); i++) {
            wm.addAP(wifiList[i].ssid.c_str(), wifiList[i].password.c_str());
        }
    }

    void addCredToList(String ssid, String password) { wifiList.push_back(WiFiCred(ssid, password)); }

    int saveWifiStringToFs(String s) {
        File dataFile = LittleFS.open("/wifi.txt", "w");
        if (!dataFile) {
            Serial.println("Failed to open config file wifi.txt for writing");
            return -1;
        }
        dataFile.print(s);
        dataFile.flush();
        dataFile.close();

        return 0;
    }

    String listToString() {
        String s = "";
        for (int i = 0; i < wifiList.size(); i++) {
            s += wifiList[i].ssid;
            s += "###";
            s += wifiList[i].password;
            s += "\n";
        }
        return s;
    }

    void dumpWifiListToFile() { saveWifiStringToFs(listToString()); }

    void parseStringAndInsertToList(String s) {
        int pos = s.indexOf("###");
        if (pos != -1) {
            String ssid = s.substring(0, pos);
            String password = s.substring(pos + 3, s.length());
            Serial.println("(" + ssid + "):(" + password + ")");
            addCredToList(ssid, password);
        }
    }

    int loadWifiFromFile() {
        wifiList.clear();
        if (LittleFS.exists("/wifi.txt")) {
            File file = LittleFS.open("/wifi.txt", "r");
            do {
                String s = file.readStringUntil('\n');
                parseStringAndInsertToList(s);
            } while (file.available());
            file.close();
            return 0;
        } else {
            Serial.println("wifi.txt not exists");
            return -1;
        }
        return 0;
    }

   public:
    WiFiConfig() {}

    void init(MeteoLog* _log, Led* _led) {
        if (isInited) {
            return;
        }
        isInited = true;
        this->log = _log;
        this->led = _led;
        loadWifiFromFile();
    }

    void saveConnectedWifiData(String ssid, String password) {
        bool exists = false;
        bool changed = false;

        for (unsigned int i = 0; i < wifiList.size(); i++) {
            if (wifiList[i].ssid == ssid) {
                exists = true;
                if (wifiList[i].password != password) {
                    wifiList[i].password = password;
                    changed = true;
                }
                break;
            }
        }

        if (!exists) {
            addCredToList(ssid, password);
            changed = true;
        }

        if (changed) {
            dumpWifiListToFile();
        }
    }

    void removeCredByInd(int ind) {
        if (ind < wifiList.size()) {
            wifiList.erase(wifiList.begin() + ind);
            dumpWifiListToFile();
        }
    }

    String getWiFiListStringForLog() {
        String ret = "";
        for (int i = 0; i < wifiList.size(); i++) {
            WiFiCred cred = wifiList[i];
            String lastChar = "";
            if (cred.password.length() > 0) {
                lastChar = cred.password.charAt(cred.password.length() - 1);
            }
            ret += String(i) + ":" + cred.ssid + ":" + lastChar;
            if (i < wifiList.size() - 1) {
                ret += ",";
            }
        }
        ret += "$";
        return ret;
    }

    static bool isConnected() { return WiFi.waitForConnectResult() == WL_CONNECTED; }

    static bool connectSavedWifi(String deviceName) {
        Serial.println("[WIFI] Trying to connect saved: " + WiFi.SSID());
        WiFi.mode(WIFI_STA);
        WiFi.hostname("ESP-" + deviceName);
        WiFi.begin();
        if (!isConnected()) {
            Serial.println("[WIFI] Connection Failed!");
            return false;
        }
        return true;
    }

    void connectMultiIfNeeded() {
        if (isConnected()) {
            return;
        }

        led->blink(5);
        delay(500);
        log->add("[WIFI] multi prepare");
        log->add("[WIFI] persistent before multi: " + String(WiFi.getPersistent()));

        String oldSsid = WiFi.SSID();
        String oldPassword = WiFi.psk();
        WiFi.persistent(false);
        WiFi.disconnect();

        ESP8266WiFiMulti wm;
        insertCredsIntoWm(wm);

        wl_status_t status = wm.run(8000);
        if (status == WL_CONNECTED) {
            led->blink(2);
            log->add("[WIFI] multi connected");
            String newSsid = WiFi.SSID();
            String newPassword = WiFi.psk();

            // save password to flash
            if (oldSsid != newSsid || oldPassword != newPassword) {
                WiFi.persistent(true);
                // WiFi.disconnect(true);
                WiFi.begin(newSsid, newPassword);
                WiFi.persistent(false);
                log->add("[WIFI] multi save new SSID: " + newSsid);
            }
        } else {
            led->blink(3);
            // restore saved password
            log->add("[WIFI] multi restore old SSID: " + oldSsid);
            WiFi.begin(oldSsid, oldPassword);
        }

        log->add("[WIFI] persistent after multi: " + String(WiFi.getPersistent()));
        log->add("[WIFI] SSID after multi: " + WiFi.SSID());
    }

    void startPortal() {
        if (!wifiManagerIsInited) {
            wifiManager = new WiFiManager();
            wifiManagerIsInited = true;
        }

        wifiManager->resetSettings();
        wifiManager->setConfigPortalTimeout(240);
        wifiManager->startConfigPortal("NodeMCU-free0u");
    }
};

#endif
