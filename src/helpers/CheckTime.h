#ifndef CHECK_TIME_H
#define CHECK_TIME_H

class CheckTime {
   private:
    unsigned long sendSensorToServer = 0;
    unsigned long testButtonWater = 0;
    unsigned long timeIrmsSumSpend = 0;
    unsigned long time433Send = 0;
    unsigned long timeButtonPress = 0;
    unsigned long* sensorsUpdateTime;

   public:
    CheckTime() {}

    void initSensorUpdateTime(int sensorsCount) {
        sensorsUpdateTime = new unsigned long[sensorsCount];
        for (int i = 0; i < sensorsCount; i++) {
            sensorsUpdateTime[i] = 0;
        }
    }

    bool checkSendSensorToServer(unsigned long delay) { return checkTime(sendSensorToServer, delay); }
    bool checkButtonWater(unsigned long delay) { return checkTime(testButtonWater, delay); }
    bool checkIrmsSumSpend(unsigned long delay) { return checkTime(timeIrmsSumSpend, delay); }
    bool check433Send(unsigned long delay) { return checkTime(time433Send, delay); }
    bool checkButtonPress(unsigned long delay) { return checkTime(timeButtonPress, delay); }
    bool checkSensorByInd(int ind, unsigned long delay) { return checkTime(sensorsUpdateTime[ind], delay); }

   private:
    bool checkTime(unsigned long& ts, unsigned long delay) {
        if (millis() - ts > delay) {
            ts = millis();
            return true;
        }
        return false;
    }
};

#endif