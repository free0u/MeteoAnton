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

    SensorsData() {}
};

#endif