#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"
#include <NtpClientLib.h>

OTAUpdate otaUpdate;

#define BUTTON D7
int cnt = 0;

#include "SensorDallasTemp.h"
#include "BME280.h"
#include "DHTSensor.h"
#include "OLED.h"
#include "MeteoLog.h"
#include "SensorsData.h"
#include "Led.h"

SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
long bootTime;

void setup() {
    Serial.begin(115200);
    bootTime = now();

    // setup pins
    pinMode(BUTTON, INPUT);

    // turn on LED until connected
    led.on();

    // setup log and OLED
    meteoLog = new MeteoLog();
    oled = new OLED(meteoLog);
    oled->showMessage("Booting...");

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    oled->showMessage("Trying to connect WiFi");
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // DS18B20 temperature
    oled->showMessage("DS18B20 init...");
    temp = new SensorDallasTemp();
    oled->showMessage("DS18B20 init... Done");

    // BME280
    oled->showMessage("BME280 init...");
    bme = new BME280();
    oled->showMessage("BME280 init... Done");

    // DHT11
    oled->showMessage("DHT11 init...");
    dht = new DHTSensor();
    oled->showMessage("DHT11 init... Done");

    // NTP
    oled->showMessage("NTP init...");
    NTP.begin("pool.ntp.org", 5);
    NTP.setInterval(63);
    oled->showMessage("NTP init... Done");

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);
}

int oledState = 2;
int oledStateNum = 3;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int EMPTY = 4;

long timeButtonPress = 0;
long sensorsDataUpdated = 0;

SensorsData sensorsData;
float dsTemp;
float dhtHum;
float bmeTemp;
float bmePressure;
float bmeHum;

void loop() {
    otaUpdate.handle();

    if (millis() - sensorsDataUpdated > 2000) {
        sensorsDataUpdated = millis();

        dsTemp = temp->printTemperature();
        bmeTemp = bme->temperature();
        bmePressure = bme->pressure();
        bmeHum = bme->humidity();
        dhtHum = dht->humidity();

        meteoLog->add(String(bmeHum) + " %");
        sensorsData.dsTemp = dsTemp;
        sensorsData.bmeHum = bmeHum;
        sensorsData.bmePressure = bmePressure;
        sensorsData.dhtHum = dhtHum;
    }

    // display.setTextAlignment(TEXT_ALIGN_LEFT);
    // display.drawString(0, 0, String(tempC));
    // display.display();

    meteoLog->add(String(cnt++));

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(cnt++, NTP.getTimeStr());
    } else if (oledState == LOG) {
        oled->log();
    }

    // BME 280 begin
    Serial.print("Temperature = ");
    Serial.print(bmeTemp);
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print(bmePressure);
    Serial.println(" mmHg");
    Serial.print("BME Humidity = ");
    Serial.print(bmeHum);
    Serial.println(" %");
    // BME 280 end

    // DHT11 BEGIN
    Serial.print("DHT11 hum: ");
    Serial.println(dhtHum);
    // DHT11 END

    Serial.println("Start time: " + String(bootTime) + " now(): " + String(now()));
    // Serial.println("");
    // Serial.println("");
    // Serial.print(" ");
    // Serial.print(NTP.getTimeDateString());
    // Serial.print(" ");
    // Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    // Serial.print("WiFi is ");
    // Serial.print(WiFi.isConnected() ? "connected" : "not connected");
    // Serial.print(". ");
    // Serial.print("Uptime: ");
    // Serial.print(NTP.getUptimeString());
    // Serial.print(" since ");
    // Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());
    // Serial.println("");
    // Serial.println("");

    delay(1000);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        Serial.println("Button on");

        if (millis() - timeButtonPress > 500) {
            timeButtonPress = millis();
            oledState = (oledState + 1) % oledStateNum;
            Serial.println("OLED state: " + String(oledState));
        }
    }

    // IF button LONG PRESS and UPTIME smaller 1 minute
    //     start WiFi Manager

    // IF OLED works longer 1 minute
    //     OLED state -> OFF
    // IF button PUSHED
    //     OLED state -> NEXT
}
