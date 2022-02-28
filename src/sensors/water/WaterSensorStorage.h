#ifndef WATER_SENSOR_STORAGE_H
#define WATER_SENSOR_STORAGE_H

#include "LittleFS.h"

class WaterSensorStorage {
   private:
    bool isInited = false;
    MeteoLog* log;

    int pin;
    float waterSpent;
    int waterButtonState = -1;

    float getFromFs() {
        if (LittleFS.exists("/water.txt")) {
            File file = LittleFS.open("/water.txt", "r");
            float value = file.parseFloat();
            file.close();
            return value;
        } else {
            Serial.println("water.txt not exists");
            return 0;
        }
    }

    float saveToFs(float value) {
        File dataFile = LittleFS.open("/water.txt", "w");
        if (!dataFile) {
            Serial.println("Failed to open config file water.txt for writing");
            return -10;
        }
        dataFile.println(value);
        dataFile.flush();
        dataFile.close();

        return value;
    }

   public:
    WaterSensorStorage() {}
    void init(int _pin, MeteoLog* _log) {
        if (isInited) {
            return;
        }

        pin = _pin;
        log = _log;
        waterSpent = getFromFs();
        pinMode(pin, INPUT);
        isInited = true;
    }

    float getWaterSpent() { return waterSpent; }

    void processInterval() {
        int newButtonState = digitalRead(pin);
        if (waterButtonState == -1) {
            waterButtonState = newButtonState;
        } else {
            if (waterButtonState != newButtonState) {
                delay(50);
                newButtonState = digitalRead(pin);
                if (waterButtonState != newButtonState) {
                    waterButtonState = newButtonState;

                    waterSpent += 5;
                    log->add("===== Water Sensor: add 5 liters. total: " + String(waterSpent));

                    saveToFs(waterSpent);
                }
            }
        }
        // log->add("===== Water Sensor button: " + String(newButtonState));
        // log->add("===== Water Sensor total: " + String(waterSpent));
    }
};

#endif