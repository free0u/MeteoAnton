#ifndef CHECK_TIME_H
#define CHECK_TIME_H

class CheckTime {
   private:
    unsigned long sendSensorToServer = 0;

   public:
    CheckTime() {}

    // void init() {}

    bool checkSendSensorToServer(unsigned long delay) { return checkTime(sendSensorToServer, delay); }

   private:
    bool checkTime(unsigned long& ts, unsigned long delay) {
        if (millis() - ts > delay) {
            ts = millis();
            return true;
        }
        return true;
    }
};

#endif