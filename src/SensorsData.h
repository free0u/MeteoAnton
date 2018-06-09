#ifndef SensorsData_h
#define SensorsData_h

class SensorsData {
  public:
    float dsTempOne;
    float dsTempTwo;
    float dhtHum;
    float bmeHum;
    float bmePressure;

    SensorsData() {
        dsTempOne = NAN;
        dsTempTwo = NAN;
        dhtHum = NAN;
        bmeHum = NAN;
        bmePressure = NAN;
    }
};

#endif