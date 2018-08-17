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
    static const int SENSORS_COUNT = 8;
    Sensor *sensors[SENSORS_COUNT] = {&dsTempOne,   &dsTempTwo, &dhtHum,  &bmeHum,
                                      &bmePressure, &co2,       &co2uart, &uptimeMinutes};

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
    Sensor dsTempOne;
    Sensor dsTempTwo;
    Sensor dhtHum;
    Sensor bmeHum;
    Sensor bmePressure;
    Sensor co2;
    Sensor co2uart;
    Sensor uptimeMinutes;
    String sensorsNames;

    SensorsData()
        : dsTempOne("temp_in"), dsTempTwo("temp_out"), dhtHum("hum_out"), bmeHum("hum_in"), bmePressure("pressure"),
          co2("co2"), co2uart("co2uart"), uptimeMinutes("uptime") {
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
        return res;
    }
};

#endif