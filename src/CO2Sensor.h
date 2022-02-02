#ifndef CO2_SENSOR_h
#define CO2_SENSOR_h

#include <MHZ19_uart.h>

#define PWM D8

int prevVal = LOW;
long th, tl, h, l, ppm = 397;
long ppmUpdateTime = -1000000;

void PWM_ISR() {
    long tt = millis();
    int val = digitalRead(PWM);

    if (val == HIGH) {
        if (val != prevVal) {
            h = tt;
            tl = h - l;
            prevVal = val;
        }
    } else {
        if (val != prevVal) {
            l = tt;
            th = l - h;
            prevVal = val;
            ppm = 5000 * (th - 2) / (th + tl - 4);
            ppmUpdateTime = tt;
        }
    }
}

class CO2Sensor {
   private:
    const int rx_pin = D1;  // Serial rx pin no
    const int tx_pin = D2;  // Serial tx pin no
    MHZ19_uart *mhz19;

   public:
    CO2Sensor() {
        prevVal = LOW;
        ppm = 0;
        ppmUpdateTime = -1;

        mhz19 = new MHZ19_uart();
        mhz19->begin(rx_pin, tx_pin);
        mhz19->setAutoCalibration(false);
        // mhz19->setAutoCalibration(false);
        mhz19->setRange(true);
        attachCo2Interrupt();
    }
    void attachCo2Interrupt() { attachInterrupt(digitalPinToInterrupt(PWM), PWM_ISR, CHANGE); }

    void detachCo2Interrupt() { detachInterrupt(PWM); }

    int getPpmPwm() { return ppm * 1; }
    long getPpmUpateTime() { return ppmUpdateTime; }

    int getPpmUart() { return mhz19->getPPM() * 1; }

    void setZero() { mhz19->calibrateZero(); }
};

#endif
