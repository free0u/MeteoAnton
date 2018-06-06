#ifndef SensorsData_h
#define SensorsData_h

class SensorsData {
  public:
    float dsTemp;
    float dhtHum;
    float bmeHum;
    float bmePressure;

    SensorsData() {
        dsTemp = NAN;
        dhtHum = NAN;
        bmeHum = NAN;
        bmePressure = NAN;
    }
};

#endif