#include <DallasTemperature.h>
#include <OneWire.h>
// #include <HardwareSerial.h>

#define ONE_WIRE_BUS D1
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

    float tempIsCorrect(float temp) { return -100 < temp && temp < 100; }

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

    // first 28 61 64 11 8D 96 46 D3
    // second 28 61 64 11 BD D8 52 B6
    DeviceAddress addrOne = {0x28, 0x61, 0x64, 0x11, 0x8D, 0x96, 0x46, 0xD3};
    DeviceAddress addrTwo = {0x28, 0x61, 0x64, 0x11, 0xBD, 0xD8, 0x52, 0xB6};

    float temperatureOne() { return temperatureByAddr(addrOne); }

    float temperatureTwo() { return temperatureByAddr(addrTwo); }

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