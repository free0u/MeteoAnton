#include <DallasTemperature.h>
#include <OneWire.h>
// #include <HardwareSerial.h>

// #define ONE_WIRE_BUS D6 // 1
#define ONE_WIRE_BUS D6 // 2
#define TEMPERATURE_PRECISION 9

class SensorDallasTemp {
  private:
    OneWire *oneWire;
    DallasTemperature *sensors;

    int numberOfDevices;             // Number of temperature devices found
    DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

    // function to print a device address
    void printAddress(DeviceAddress deviceAddress) {
        for (uint8_t i = 0; i < 8; i++) {
            if (deviceAddress[i] < 16)
                Serial.print("0");
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
        sensors->requestTemperatures(); // Send the command to get temperatures
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
    SensorDallasTemp() {
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

    // addrOne // 1
    // addr2 // 2
    float temperatureOne() { return temperatureByAddr(addr2); }

    float temperatureTwo() { return temperatureByAddr(addrTwo); }

    float temperatureThree() { return temperatureByAddr(addrThree); }

    float printTemperature2() {
        sensors->setWaitForConversion(false);
        sensors->requestTemperatures(); // Send the command to get temperatures
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