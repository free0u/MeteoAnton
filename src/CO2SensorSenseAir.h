#ifndef CO2_SENSOR_SENSEAIR_h
#define CO2_SENSOR_SENSEAIR_h

// #include <MHZ19_uart.h>
#include "Arduino.h"
#include "SoftwareSerial.h"

class CO2SensorSenseAir {
   private:
    // MHZ19_uart *mhz19;
    SoftwareSerial *Sensor;

    static const int REQUEST_CNT = 8;
    // <FE> <04> <00> <03> <00> <01> <D5> <C5>
    uint8_t readCO2[8] = {0xFE, 0X04, 0X00, 0X03, 0X00, 0X01, 0XD5, 0xC5};

    //<FE> <03> <00> <1F> <00> <01> <A1> <C3>
    // uint8_t readABC[8] = {0xFE, 0X03, 0X00, 0X1F, 0X00, 0X01, 0XA1, 0xC3};
    // uint8_t readCO2[REQUEST_CNT] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; // old
    uint8_t response[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    // calibrate
    // clear
    // <FE> <06> <00> <00> <00> <00> <9D> <C5>
    // uint8_t calibClear[8] = {0xFE, 0X06, 0X00, 0X00, 0X00, 0X00, 0X9D, 0xC5};
    // run calib
    // <FE> <06> <00> <01> <7C> <06> <6C> <C7>
    // uint8_t calibRun[8] = {0xFE, 0X06, 0X00, 0X01, 0X7C, 0X06, 0X6C, 0xC7};

    // multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
    int valMultiplier = 1;

    int rx, tx;

   public:
    CO2SensorSenseAir() {}
    void init(int _rx, int _tx) {  // D1 D2
        rx = _rx;
        tx = _tx;
        Sensor = new SoftwareSerial(rx, tx);
        Sensor->begin(9600);
    }

    float read() {
        writeCommand(readCO2);
        return getValue(response);
    }

    // int getABC() {
    // writeCommand(readABC);
    // return getValue(response);
    // }

    // bool calib() {
    //     int cnt = 0;
    //     while (cnt < 3) {
    //         if (writeCommandCalib(calibClear)) {
    //             break;
    //         }
    //         cnt++;
    //         delay(1000);
    //     }
    //     if (cnt == 3) {
    //         Serial.println("Cant clear register");
    //         return false;
    //     }

    //     cnt = 0;
    //     while (cnt < 3) {
    //         if (writeCommandCalib(calibRun)) {
    //             break;
    //         }
    //         cnt++;
    //         delay(1000);
    //     }

    //     if (cnt == 3) {
    //         Serial.println("Cant run calibration");
    //         return false;
    //     }

    //     return true;
    // }

   private:
    // bool writeCommandCalib(uint8_t cmd[]) {
    //     SoftwareSerial *hserial;
    //     hserial = new SoftwareSerial(rx, tx);

    //     hserial->begin(9600);

    //     bool res = false;

    //     int nbBytesSent = hserial->write(cmd, REQUEST_CNT);
    //     hserial->flush();
    //     if (nbBytesSent != 8) {
    //         Serial.println("SenseAir: Error, nb bytes sent != 9 : " + String(nbBytesSent));
    //     } else {
    //         Serial.println("Sent 8 bytes");
    //     }

    //     if (response != NULL) {
    //         memset(response, 0, 8);
    //         long start = millis();
    //         int counter = 0;
    //         while (((millis() - start) < 400) && (counter < 8)) {
    //             if (hserial->available() > 0) {
    //                 int b = hserial->read();
    //                 response[counter++] = b;
    //                 Serial.print(response[counter - 1]);
    //                 Serial.print(" ");
    //             } else {
    //                 yield();
    //                 delay(10);
    //             }
    //         }
    //         // Serial.println("");

    //         if (counter < 8) {
    //             Serial.println("SenseAir: Error, timeout while trying to read, counter = " + String(counter));
    //         } else {
    //             Serial.println("Counter = " + String(counter));
    //             res = true;
    //         }
    //     }

    //     delete hserial;
    //     return res;
    // }
    void writeCommand(uint8_t cmd[]) {
        SoftwareSerial *hserial = Sensor;
        // hserial = new SoftwareSerial(rx, tx);
        // hserial->begin(9600);

        int nbBytesSent = hserial->write(cmd, REQUEST_CNT);
        hserial->flush();
        if (nbBytesSent != 8) {
            Serial.println("SenseAir: Error, nb bytes sent != 9 : " + String(nbBytesSent));
        } else {
            Serial.println("Sent 8 bytes");
        }

        if (response != NULL) {
            memset(response, 0, 7);
            long start = millis();
            int counter = 0;
            while (((millis() - start) < 400) && (counter < 7)) {
                if (hserial->available() > 0) {
                    int b = hserial->read();
                    response[counter++] = b;
                    Serial.print(response[counter - 1]);
                    Serial.print(" ");
                } else {
                    yield();
                    delay(10);
                }
            }
            // Serial.println("");

            if (counter < 7) {
                Serial.println("SenseAir: Error, timeout while trying to read, counter = " + String(counter));
            } else {
                Serial.println("Counter = " + String(counter));
            }
        }

        // delete hserial;
    }

    // void sendRequest(uint8_t packet[]) {
    //     int cnt = 0;
    //     while (!Sensor->available() && (cnt++ > 10)) // keep sending request until we start to get a response
    //     {
    //         Sensor->write(readCO2, 7);
    //         delay(50);
    //     }

    //     int timeout = 0;                // set a timeoute counter
    //     while (Sensor->available() < 7) // Wait to get a 7 byte response
    //     {
    //         timeout++;
    //         if (timeout > 10) // if it takes to long there was probably an error
    //         {
    //             while (Sensor->available()) // flush whatever we have
    //                 Sensor->read();

    //             break; // exit and try again
    //         }
    //         delay(50);
    //         Serial.println("read");
    //     }

    //     for (int i = 0; i < 7; i++) {
    //         response[i] = Sensor->read();
    //     }

    //     for (int i = 0; i < 7; i++) {
    //         Serial.print(response[i]);
    //         Serial.print(" ");
    //     }
    // }

    unsigned long getValue(uint8_t packet[]) {
        int high = packet[3];  // high byte for value is 4th byte in packet in the packet
        int low = packet[4];   // low byte for value is 5th byte in the packet

        unsigned long val = high * 256 + low;  // Combine high byte and low byte with this formula to get value
        return val * valMultiplier;
    }
};

#endif
