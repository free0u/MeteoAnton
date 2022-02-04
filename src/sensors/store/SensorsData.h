#ifndef SensorsData_h
#define SensorsData_h

#include <TimeLib.h>

#include "system/DevicesConfig.h"
#include "system/Timeouts.h"

class Sensor {
   private:
    long ts;
    float value;
    String name;

   public:
    Sensor() {}

    void init(String _name) {
        value = NAN;
        ts = -1e9;
        name = _name;
    }

    void set(float v, long t) {
        if (!isnan(v)) {
            value = v;
            ts = t;
        }
    }

    float getIfUpdated() {
        long present = 1543176962;                                 // 11/25/2018 @ 8:16pm (UTC)
        if (ts < present || now() < present || ts > 2082758400) {  // 01/01/2036 @ 12:00am (UTC)
            return NAN;
        }
        if (now() - ts < SENSOR_CORRECT_TIMEOUT) {
            return value;
        }
        return NAN;
    }

    long getTime() { return ts; }

    String getName() { return name; }
};

class SensorsData {
   private:
    int sensorsCount;

    String getSensorsNames() {
        String res = "";
        for (int i = 0; i < sensorsCount; i++) {
            if (i > 0) {
                res += ';';
            }
            res += sensors[i].getName();
        }
        return res;
    }

   public:
    Sensor* sensors;
    String sensorsNames;

    SensorsData() {}

    void init(SensorConfig* sensorsConfig, int count) {
        this->sensorsCount = count;
        sensors = new Sensor[count];
        for (int i = 0; i < count; i++) {
            sensors[i].init(sensorsConfig[i].field_name);
        }
        sensorsNames = getSensorsNames();
    }

    void init() { sensorsNames = getSensorsNames(); }

    String serialize() {
        long timeNow = now();
        String res = String(timeNow);
        for (int i = 0; i < sensorsCount; i++) {
            float value = sensors[i].getIfUpdated();
            if (!isnan(value)) {
                long ts = sensors[i].getTime();
                res += ';';
                res += String(value);
                res += ';';
                res += String(timeNow - ts);
            } else {
                res += ";;";
            }
        }
        Serial.println("Serialize: " + res);
        return res;
    }
};

#endif