#ifndef SENSORS_CACHE_H
#define SENSORS_CACHE_H

#include "SensorsData.h"

class SensorsCache {
  private:
    bool cacheIsEmpty;
    int cachedCount;
    static const int ONE_MEASURE_LEN = 70;

  public:
    SensorsCache() {
        if (SPIFFS.exists("/data.json")) {
            File file = SPIFFS.open("/data.json", "r");
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
        return SPIFFS.remove("/data.json");
    }

    bool add(SensorsData *data) {
        File dataFile = SPIFFS.open("/data.json", "a");
        if (!dataFile) {
            Serial.println("Failed to open config file for writing");
            return false;
        }

        if (cachedCount > 8000) {
            return false;
        }

        dataFile.print(data->serialize());
        dataFile.print("_");
        cacheIsEmpty = false;
        cachedCount++;

        return true;
    }

    bool sendCache(String &sensorsNames) {
        File dataFile = SPIFFS.open("/data.json", "r");
        if (!dataFile) {
            Serial.println("Failed to open config file");
            return false;
        }

        HTTPClient http;
        http.begin("***REMOVED***");
        http.setTimeout(5000);
        http.addHeader("Sensors-Names", sensorsNames);
        int statusCode = http.sendRequest("POST", &dataFile, dataFile.size());
        http.end();

        if (statusCode == 200) {
            clear();
        }

        return true;
    }

    int getCachedCount() { return cachedCount; }

    bool printFile() {
        File dataFile = SPIFFS.open("/data.json", "r");
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