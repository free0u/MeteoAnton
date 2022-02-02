#ifndef EMONLIBSensor_h
#define EMONLIBSensor_h

#include "EmonLib.h"

class EmonLibSensor {
   private:
    bool isInit = false;
    float ICAL;
    float voltage;
    EnergyMonitor emon;

   public:
    EmonLibSensor() { isInit = false; }
    void init(float _ICAL, float _voltage) {
        if (isInit) {
            return;
        }
        ICAL = _ICAL;
        voltage = _voltage;
        isInit = true;

        emon.current(A0, ICAL);
    }
    float Irms() {
        float I = emon.calcIrms(1480);
        return I;
    }
    float power() { return voltage * Irms(); }
};

#endif
