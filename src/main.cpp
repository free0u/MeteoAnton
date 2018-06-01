#include <OneWire.h>
#include <DallasTemperature.h>
#include <SensorDallasTemp.h>



SensorDallasTemp* temp;

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  //SensorDallasTemp temp2;
  //SensorDallasTemp temp = SensorDallasTemp();

    delay(2000);
    
    temp = new SensorDallasTemp();

}



void loop(void)
{ 
    temp->printTemperature();
  delay(2000);
}

