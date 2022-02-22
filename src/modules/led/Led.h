#ifndef LED_H
#define LED_H

class Led {
   private:
    uint8_t pin;
    bool ledIsOn;

    void on(int x) {
        analogWrite(pin, x);
        ledIsOn = true;
    }

   public:
    Led() {}

    void init(uint8_t _pin) {
        pin = _pin;
        pinMode(pin, OUTPUT);
        off();
    }

    void on() { on(254); }

    void off() {
        digitalWrite(pin, HIGH);
        ledIsOn = false;
    }

    void change() {
        if (ledIsOn) {
            off();
        } else {
            on();
        }
    }
};

#endif