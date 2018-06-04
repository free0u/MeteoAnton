#ifndef SensorsData_h
#define SensorsData_h

class SensorsData {
  public:
    float dsTemp;
    float dhtHum;
    float bmeHum;
    float bmePressure;

    SensorsData() {
        dsTemp = 1;
        dhtHum = 2;
        bmeHum = 3;
        bmePressure = 4;
    }
};

#endif