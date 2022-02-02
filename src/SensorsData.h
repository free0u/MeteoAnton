#ifndef SensorsData_h
#define SensorsData_h

#include <TimeLib.h>

#include "DevicesConfig.h"
#include "Timeouts.h"

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
    // static const int SENSORS_COUNT = 7;
    // Sensor* sensors[SENSORS_COUNT] = {&dsTempIn1, &dsTempIn2, &dsTempOut, &dhtHum, &co2, &uptime, &buildVersion};
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
    // Sensor co2;
    // Sensor uptime;
    // Sensor buildVersion;

    // Sensor dsTempIn1;
    // Sensor dsTempIn2;
    // Sensor dsTempOut;
    // Sensor dhtHum;
    // Sensor bmeHum;
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

    void init() {
        // dsTempIn1.init("temp_in");
        // dsTempIn2.init("temp_in_bak");
        // dsTempOut.init("temp_out");
        // dhtHum.init("hum_in");
        // co2.init("co2");
        // uptime.init("uptime");
        // buildVersion.init("build_version");
        sensorsNames = getSensorsNames();
    }

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