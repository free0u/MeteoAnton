#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS D1
#define TEMPERATURE_PRECISION 9

class SensorDallasTemp {
  private:
  public:
    OneWire *oneWire;
    DallasTemperature *sensors;

    int numberOfDevices;             // Number of temperature devices found
    DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

    SensorDallasTemp();
    float printTemperature();
};