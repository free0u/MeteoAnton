#ifndef ELECTRO_SENSOR_STORAGE_H
#define ELECTRO_SENSOR_STORAGE_H

#include "LittleFS.h"

class ElectroSensorStorage {
   public:
    ElectroSensorStorage() {}

    float get() {
        if (LittleFS.exists("/electro.txt")) {
            File file = LittleFS.open("/electro.txt", "r");
            float value = file.parseFloat();
            file.close();
            return value;
        } else {
            Serial.println("electro.txt not exists");
            return 0;
        }
    }

    float save(float value) {
        File dataFile = LittleFS.open("/electro.txt", "w");
        if (!dataFile) {
            Serial.println("Failed to open config file electro.txt for writing");
            return -10;
        }
        dataFile.println(value);
        dataFile.flush();
        dataFile.close();

        return get();
    }
};

#endif