#ifndef WATER_SENSOR_STORAGE_H
#define WATER_SENSOR_STORAGE_H

#include "LittleFS.h"

class WaterSensorStorage {
   public:
    WaterSensorStorage() {}

    float get() {
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

    float save(float value) {
        File dataFile = LittleFS.open("/water.txt", "w");
        if (!dataFile) {
            Serial.println("Failed to open config file water.txt for writing");
            return -10;
        }
        dataFile.println(value);
        dataFile.flush();
        dataFile.close();

        return get();
    }
};

#endif