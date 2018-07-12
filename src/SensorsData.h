#ifndef SensorsData_h
#define SensorsData_h

#include <Time.h>
#include "Timeouts.h"

class Sensor {
  private:
    long ts;
    float value;

  public:
    Sensor() {
        value = NAN;
        ts = -1e9;
    }

    void set(float v, long t) {
        value = v;
        ts = t;
    }

    float getIfUpdated() {
        if (now() - ts < SENSOR_CORRECT_TIMEOUT) {
            return value;
        }
        return NAN;
    }

    long getTime() { return ts; }
};

class SensorsData {
  public:
    Sensor dsTempOne;
    Sensor dsTempTwo;
    Sensor dhtHum;
    Sensor bmeHum;
    Sensor bmePressure;
    Sensor co2;
    Sensor co2uart;

    static const int SENSORS_COUNT = 7;

    Sensor *sensors[SENSORS_COUNT] = {&dsTempOne, &dsTempTwo, &dhtHum, &bmeHum, &bmePressure, &co2, &co2uart};

    SensorsData() {}

    String serialize() {
        long timeNow = now();
        String res = String(timeNow);
        for (int i = 0; i < SENSORS_COUNT; i++) {
            float value = sensors[i]->getIfUpdated();
            if (!isnan(value)) {
                long ts = sensors[i]->getTime();
                res += ";";
                res += String(value);
                res += ";";
                res += String(timeNow - ts);
            } else {
                res += ";;";
            }
        }
        return res;
    }
};

#endif