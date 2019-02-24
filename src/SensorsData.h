#ifndef SensorsData_h
#define SensorsData_h

#include <Time.h>
#include "Timeouts.h"

class Sensor {
  private:
    long ts;
    float value;
    String name;

  public:
    Sensor() {}

    Sensor(String _name) {
        value = NAN;
        ts = -1e9;
        name = _name;
    }

    void set(float v, long t) {
        value = v;
        ts = t;
    }

    float getIfUpdated() {
        long present = 1543176962;                                // 11/25/2018 @ 8:16pm (UTC)
        if (ts < present || now() < present || ts > 2082758400) { // 01/01/2036 @ 12:00am (UTC)
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
    static const int SENSORS_COUNT = 4;
    Sensor *sensors[SENSORS_COUNT] = {&dsTemp, &dhtHum, &co2, &uptime};

    String getSensorsNames() {
        String res = "";
        for (int i = 0; i < SENSORS_COUNT; i++) {
            if (i > 0) {
                res += ';';
            }
            res += sensors[i]->getName();
        }
        return res;
    }

  public:
    Sensor co2;
    Sensor uptime;

    Sensor dsTemp;
    Sensor dhtHum;
    Sensor bmeHum;
    String sensorsNames;

    SensorsData() : dsTemp("temp_in"), dhtHum("hum_in"), co2("co2"), uptime("uptime") {
        sensorsNames = getSensorsNames();
    }

    String serialize() {
        long timeNow = now();
        String res = String(timeNow);
        for (int i = 0; i < SENSORS_COUNT; i++) {
            float value = sensors[i]->getIfUpdated();
            if (!isnan(value)) {
                long ts = sensors[i]->getTime();
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