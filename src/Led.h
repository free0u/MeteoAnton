#ifndef LED_H
#define LED_H

#define LED D0

class Led {
  private:
    bool ledIsOn = false;

  public:
    Led() { pinMode(LED, OUTPUT); }
    void on() {
        analogWrite(LED, 300);
        ledIsOn = true;
    }
    void off() {
        analogWrite(LED, 0);
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