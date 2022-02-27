#ifndef ELECTRO_SENSOR_STORAGE_H
#define ELECTRO_SENSOR_STORAGE_H

#include "LittleFS.h"
#include "sensors/electro/EmonLibSensor.h"

class ElectroSensorStorage {
   private:
    bool isInited = false;
    EmonLibSensor* emon;
    MeteoLog* log;

    float powerSpent;
    float lastPower = 0;
    long lastTs = 0;

    float getFromFs() {
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

    float saveToFs(float value) {
        File dataFile = LittleFS.open("/electro.txt", "w");
        if (!dataFile) {
            Serial.println("Failed to open config file electro.txt for writing");
            return -10;
        }
        dataFile.println(value);
        dataFile.flush();
        dataFile.close();

        return value;
        // return getFromFs();
    }

   public:
    ElectroSensorStorage() {}

    void init(EmonLibSensor* _emon, MeteoLog* _log) {
        if (isInited) {
            return;
        }
        emon = _emon;
        log = _log;
        powerSpent = getFromFs();
        isInited = true;
    }

    float get2() { return powerSpent; }

    void processInterval() {
        long tsNow = millis();
        float curPower = emon->power();

        if (tsNow - lastTs < 6000) {
            float avgWatt = (curPower + lastPower) / 2;
            float wattSpent = avgWatt / 60 / 60 * ((tsNow - lastTs) / 1000.0);

            powerSpent += wattSpent;
            saveToFs(powerSpent);

            log->add("IRMSsum avgWatt: " + String(avgWatt));
            log->add("IRMSsum wattSpent: " + String(wattSpent));
        }

        lastTs = tsNow;
        lastPower = curPower;

        // // -------------
        // float res = electroSensorStorage.save(powerSpent);
        // meteoLog.add("electroSensorStorage init write: " + String(powerSpent) + " res: " + String(res));
        // long ts = millis();

        // meteoLog.add("IRMSsum start. Since last: " + String(ts - irmsSumSpentSmallTimestamp));

        // long st = millis();
        // float power = emonSensor.power();
        // long timeWork = millis() - st;

        // if (ts - irmsSumSpentSmallTimestamp < 6000) {
        //     float avgKw = (power + irmsSumSpentSmallValue) / 2;
        //     meteoLog.add("IRMSsum avgWatt: " + String(avgKw));
        //     float kwSpent = avgKw / 60 / 60 * ((ts - irmsSumSpentSmallTimestamp) / 1000.0);
        //     irmsSumSpentIntervalSumValue += kwSpent;

        //     powerSpent += kwSpent;

        //     // meteoLog.add("IRMSsum deltaKw: " + String(deltaKw));
        //     meteoLog.add("IRMSsum wattSpent: " + String(kwSpent));
        // }

        // irmsSumSpentSmallTimestamp = ts;
        // irmsSumSpentSmallValue = power;

        // meteoLog.add("IRMSsum, time spent: " + String(timeWork));

        // return;
    }
};

#endif