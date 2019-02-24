#ifndef LED_H
#define LED_H

#define LED D4

class Led {
  private:
    bool ledIsOn = false;

  public:
    Led() { pinMode(LED, OUTPUT); }
    void on() {
        // digitalWrite(LED, LOW);
        analogWrite(LED, 1010);
        ledIsOn = true;
    }

    void on(int x) {
        analogWrite(LED, x);
        ledIsOn = true;
    }

    void off() {
        digitalWrite(LED, HIGH);
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