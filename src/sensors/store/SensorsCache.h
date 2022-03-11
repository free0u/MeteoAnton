#ifndef SENSORS_CACHE_H
#define SENSORS_CACHE_H

#include "SensorsData.h"

class SensorsCache {
   private:
    bool cacheIsEmpty;
    int cachedCount;
    static const int ONE_MEASURE_LEN = 70;
    String sensorsApiUrl;

   public:
    SensorsCache() {}
    void init(String url) {
        this->sensorsApiUrl = url;
        clear();
        if (LittleFS.exists("/data.json")) {
            File file = LittleFS.open("/data.json", "r");
            int sz = file.size();
            file.close();

            cachedCount = sz / ONE_MEASURE_LEN + 1;
            cacheIsEmpty = false;
        } else {
            cachedCount = 0;
            cacheIsEmpty = true;
        }
    }

    bool empty() { return cacheIsEmpty; }

    bool clear() {
        cacheIsEmpty = true;
        cachedCount = 0;
        return LittleFS.remove("/data.json");
    }

    bool add(SensorsData &data) {
        File dataFile = LittleFS.open("/data.json", "a");
        if (!dataFile) {
            Serial.println("Failed to open config file for writing");
            return false;
        }

        if (cachedCount > 8000) {
            return false;
        }

        dataFile.print(data.serialize());
        dataFile.print("_");
        cacheIsEmpty = false;
        cachedCount++;

        return true;
    }

    bool sendCache(String &sensorsNames) {
        File dataFile = LittleFS.open("/data.json", "r");
        if (!dataFile) {
            Serial.println("Failed to open config file");
            clear();
            return false;
        }

        Serial.println("Sending cache data...");

        WiFiClient client;
        HTTPClient http;
        http.begin(client, sensorsApiUrl);
        http.setTimeout(10000);
        http.addHeader("Sensors-Names", sensorsNames);
        int statusCode = http.sendRequest("POST", &dataFile, dataFile.size());
        String payload = http.getString();
        Serial.println("Code: " + String(statusCode));
        Serial.println("Payload: " + payload);

        http.end();
        Serial.println("Sending data complete...");

        if (statusCode == 200) {
            clear();
        }

        return true;
    }

    int getCachedCount() { return cachedCount; }

    bool printFile() {
        File dataFile = LittleFS.open("/data.json", "r");
        if (!dataFile) {
            Serial.println("Failed to open config file");
            return false;
        }
        Serial.println("File size " + String(dataFile.size()));
        String s;
        int ind = 0;
        do {
            s = dataFile.readString();
            Serial.println("do: " + String(s.length()));
            Serial.println(String(ind++) + ": " + s);
        } while (dataFile.available() && ind < 10);

        Serial.println(String(ind) + " lines");
        return true;
    }
};

#endif