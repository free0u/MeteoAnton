#ifndef METEOLOG_H
#define METEOLOG_H

class MeteoLog {
  private:
    static const int NUM = 6;
    String messages[NUM];
    int firstMessageInd;
    bool printToSerial = true;

  public:
    MeteoLog() {
        firstMessageInd = 0;
        for (size_t i = 0; i < NUM; i++) {
            messages[i] = "";
        }
    }
    void add(String message) {
        if (printToSerial) {
            Serial.println("* " + message);
        }
        messages[firstMessageInd] = message;
        firstMessageInd++;
        firstMessageInd %= NUM;
    }
    String get(int ind) {
        ind = (firstMessageInd + ind) % NUM;
        return messages[ind];
    }
    int getCount() { return NUM; }
    void setPrintToSerial(bool print) { printToSerial = print; }
};

#endif