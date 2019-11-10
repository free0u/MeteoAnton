#include <DallasTemperature.h>
#include <OneWire.h>
// #include <HardwareSerial.h>

#define ONE_WIRE_BUS D6  // 1 opus
// #define ONE_WIRE_BUS D6 // 2 wave
#define TEMPERATURE_PRECISION 9

class SensorDallasTemp {
   private:
    OneWire *oneWire;
    DallasTemperature *sensors;

    int numberOfDevices;              // Number of temperature devices found
    DeviceAddress tempDeviceAddress;  // We'll use this variable to store a found device address

    // function to print a device address
    void printAddress(DeviceAddress deviceAddress) {
        for (uint8_t i = 0; i < 8; i++) {
            if (deviceAddress[i] < 16) Serial.print("0");
            Serial.print(deviceAddress[i], HEX);
        }
    }

    float tempIsCorrect(float temp) {
        // return true;
        return -80 < temp && temp < 80;
        return -100 < temp && temp < 100;
    }

    float temperatureByAddr(DeviceAddress deviceAddress) {
        sensors->setWaitForConversion(false);
        sensors->requestTemperatures();  // Send the command to get temperatures
        pinMode(ONE_WIRE_BUS, OUTPUT);
        digitalWrite(ONE_WIRE_BUS, HIGH);
        delay(750);

        float tempC = sensors->getTempC(deviceAddress);
        if (!tempIsCorrect(tempC)) {
            Serial.print("Cant get temp for device ");
            printAddress(deviceAddress);
            Serial.print(" is ");
            Serial.println(tempC);
            return NAN;
        }
        return tempC;
    }

   public:
    SensorDallasTemp() {}
    void init() {
        oneWire = new OneWire(ONE_WIRE_BUS);
        sensors = new DallasTemperature(oneWire);

        Serial.println("Dallas Temperature IC Control Library Demo");
        sensors->begin();
        numberOfDevices = sensors->getDeviceCount();

        Serial.print("Locating devices...");

        Serial.print("Found ");
        Serial.print(numberOfDevices, DEC);
        Serial.println(" devices.");

        // Loop through each device, print out address
        for (int i = 0; i < numberOfDevices; i++) {
            // Search the wire for address
            if (sensors->getAddress(tempDeviceAddress, i)) {
                Serial.print("Found device ");
                Serial.print(i, DEC);
                Serial.print(" with address: ");
                printAddress(tempDeviceAddress);
                Serial.println();

                Serial.print("Resolution actually set to: ");
                Serial.print(sensors->getResolution(tempDeviceAddress), DEC);
                Serial.println();
            } else {
                Serial.print("Found ghost device at ");
                Serial.print(i, DEC);
                Serial.print(" but could not detect address. Check power and cabling");
            }
        }
    }

    // one 28 72 8B 26 0A 00 00 40
    DeviceAddress addrOne = {0x28, 0x72, 0x8B, 0x26, 0x0A, 0x00, 0x00, 0x40};

    // two 28 EC BE 26 0A 00 00 B4
    DeviceAddress addrTwo = {0x28, 0xEC, 0xBE, 0x26, 0x0A, 0x00, 0x00, 0xB4};

    // three 28 6C A5 25 0A 00 00 DA
    DeviceAddress addrThree = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    DeviceAddress addr2 = {0x28, 0x6C, 0xA5, 0x25, 0x0A, 0x00, 0x00, 0xDA};

    // 28 61 64 11 8D 96 46 D3
    DeviceAddress addrChelIn1 = {0x28, 0x61, 0x64, 0x11, 0x8D, 0x96, 0x46, 0xD3};
    // 28 61 64 11 BD D8 52 B6
    DeviceAddress addrChelIn2 = {0x28, 0x61, 0x64, 0x11, 0xBD, 0xD8, 0x52, 0xB6};
    // 28 8C 99 3B 04 00 00 8F
    DeviceAddress addrChelOut = {0x28, 0x8C, 0x99, 0x3B, 0x04, 0x00, 0x00, 0x8F};

    // 28 44 04 A7 33 14 01 34
    DeviceAddress addrDino = {0x28, 0x44, 0x04, 0xA7, 0x33, 0x14, 0x01, 0x34};

    // addrOne // 1 opus
    // addr2 // 2 wave
    // addrChelIn1, addrChelIn1, // chel
    float temperatureOne() { return temperatureByAddr(addrDino); }

    float temperatureTwo() { return temperatureByAddr(addrChelIn2); }

    float temperatureThree() { return temperatureByAddr(addrChelOut); }

    float printTemperature2() {
        sensors->setWaitForConversion(false);
        sensors->requestTemperatures();  // Send the command to get temperatures
        pinMode(ONE_WIRE_BUS, OUTPUT);
        digitalWrite(ONE_WIRE_BUS, HIGH);
        delay(750);
        for (int i = 0; i < numberOfDevices; i++) {
            if (sensors->getAddress(tempDeviceAddress, i)) {
                float tempC = sensors->getTempC(tempDeviceAddress);
                Serial.print("DS18 Temp C: ");
                Serial.print(tempC);

                Serial.print(" for device: ");
                printAddress(tempDeviceAddress);
                Serial.println();

                return tempC;
            }
            // else ghost device! Check your power requirements and cabling
        }
    }
};