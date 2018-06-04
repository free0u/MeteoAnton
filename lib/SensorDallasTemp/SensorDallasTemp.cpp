#include <HardwareSerial.h>

#include "SensorDallasTemp.h"

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

SensorDallasTemp::SensorDallasTemp() {
    oneWire = new OneWire(ONE_WIRE_BUS);
    sensors = new DallasTemperature(oneWire);

    Serial.println("Dallas Temperature IC Control Library Demo");
    sensors->begin();
    numberOfDevices = sensors->getDeviceCount();
    numberOfDevices = 1;

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
};

// function to print the temperature for a device
float SensorDallasTemp::printTemperature() {
    sensors->requestTemperatures(); // Send the command to get temperatures
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