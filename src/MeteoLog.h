#ifndef METEOLOG_H
#define METEOLOG_H

class MeteoLog {
   private:
    static const int NUM = 100;
    String* messages;
    int indexToWrite;
    int indexToReadFrom;
    int messagesCount = 0;
    bool printToSerial = true;

    int count = 0;

   public:
    MeteoLog() {}

    void init() {
        messages = new String[NUM];

        indexToWrite = 0;
        indexToReadFrom = 0;
        for (size_t i = 0; i < NUM; i++) {
            messages[i] = "";
        }
    }
    void add(String message) {
        if (printToSerial) {
            Serial.println("* " + message);
        }
        messages[indexToWrite] = String(millis()) + "#" + String(++count) + " " + message;
        // messages[indexToWrite] = String(millis()) + " " + String(++count);
        indexToWrite++;
        indexToWrite %= NUM;
        messagesCount++;
        if (messagesCount > NUM) {
            messagesCount = NUM;
            indexToReadFrom++;
            indexToReadFrom %= NUM;
        }
    }
    void add(int x) { add(String(x)); }
    void add(String a, String b) { add(a + b); }
    void add(String a, String b, String c) { add(a + b + c); }
    void add(String a, String b, String c, String d) { add(a + b + c + d); }

    String get(int ind) {
        ind = (indexToReadFrom + ind) % NUM;
        return messages[ind];
    }

    int sendLog(String deviceName, String url) {
        Serial.println("Trying to send logs");
        Serial.println("messages count: " + String(messagesCount));
        String resLog = "";
        for (int i = 0; i < messagesCount; i++) {
            // Serial.println("get(i) = " + get(i));
            resLog += get(i);
            if (i < messagesCount - 1) {
                resLog += '$';
            }
        }

        // for (int i = 0; i < NUM; i++) {
        //     Serial.print(messages[i]);
        //     Serial.print(';');
        // }
        // Serial.println();

        Serial.println("len of log: " + String(resLog.length()));
        // Serial.println("log: " + resLog);

        url += ("?device=" + deviceName);
        url += ("&time=" + String(millis()));

        WiFiClient client;
        HTTPClient http;
        http.begin(client, url);
        http.setTimeout(10000);
        int statusCode = http.sendRequest("POST", resLog);
        Serial.println("Code: " + String(statusCode));

        if (statusCode == 200) {
            messagesCount = 0;
            indexToWrite = 0;
            indexToReadFrom = 0;
            Serial.println("Log sent");
        }

        http.end();

        return 0;
    }

    int getCount() { return NUM; }
    void setPrintToSerial(bool print) { printToSerial = print; }
};

#endif