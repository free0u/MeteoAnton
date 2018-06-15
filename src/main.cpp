#include "FS.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"
#include <NtpClientLib.h>
#include <RtcDS3231.h>
#include <Wire.h>
RtcDS3231<TwoWire> Rtc(Wire);

OTAUpdate otaUpdate;

#define BUTTON D4
int cnt = 0;

#include "BME280.h"
#include "DHTSensor.h"
#include "Led.h"
#include "MeteoLog.h"
#include "OLED.h"
#include "SensorDallasTemp.h"
#include "SensorsData.h"

SensorDallasTemp *temp;
BME280 *bme;
DHTSensor *dht;
OLED *oled;
MeteoLog *meteoLog;
Led led;
long bootTime;

void scan() {
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        } else if (error == 4) {
            Serial.print("Unknow error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");

    // delay(5000); // wait 5 seconds for next scan
}

#define countof(a) (sizeof(a) / sizeof(a[0]))
void printDateTime(const RtcDateTime &dt) {
    char datestring[20];

    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u/%04u %02u:%02u:%02u"), dt.Month(), dt.Day(), dt.Year(),
               dt.Hour(), dt.Minute(), dt.Second());
    Serial.print(datestring);
}

String rtcToString(const RtcDateTime &dt) {
    char datestring[20];

    snprintf_P(datestring, countof(datestring), PSTR("%02u:%02u:%02u"), dt.Hour(), dt.Minute(), dt.Second());
    return String(datestring);
}

int oledState = 2;
int oledStateNum = 3;
const int SENSORS = 0;
const int NETWORK = 1;
const int LOG = 2;
const int EMPTY = 4;

#include <MHZ19_uart.h>
const int rx_pin = D1; // Serial rx pin no
const int tx_pin = D2; // Serial tx pin no
MHZ19_uart mhz19;

void setup() {
    Serial.begin(115200);

    bootTime = now();

    Wire.begin();

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
    // @todo: start configurator by pressing button, elsewhere just stay
    // disconnected
    oled->showMessage("Trying to connect WiFi");
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    led.off();
    oled->showMessage("WiFi connected");

    // configure and start OTA update server
    oled->showMessage("OTA server init...");
    otaUpdate.setup();
    oled->showMessage("OTA server init... Done");

    // CO2
    oled->showMessage("CO2 init...");
    mhz19.begin(rx_pin, tx_pin);
    // mhz19.setAutoCalibration(false);
    oled->showMessage("CO2 init... Done");

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

    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid()) {
        Serial.println("RTC lost confidence in the DateTime!");
        // Rtc.SetDateTime(compiled);
    }

    if (!Rtc.GetIsRunning()) {
        Serial.println("RTC was not actively running, starting now");
        // Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        // Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        Serial.println("RTC is newer than compile time. (this is expected)");
    } else if (now == compiled) {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    oledState = SENSORS;
}

long timeButtonPress = 0;

SensorsData sensorsData;

#define SENSORS_TIMEOUT 30000
#define DHT_TIMEOUT 10000

long sensorsDataUpdated = -1e9;
long dhtSensorUpdated = -1e9;
void tryUpdateSensors() {
    float dsTempOne;
    float dsTempTwo;
    float dhtHum;
    float bmePressure;
    float bmeHum;

    if (millis() - sensorsDataUpdated > SENSORS_TIMEOUT) {
        sensorsDataUpdated = millis();

        meteoLog->add("Reading sensors...");
        dsTempOne = temp->temperatureOne();
        dsTempTwo = temp->temperatureTwo();
        bmePressure = bme->pressure();
        bmeHum = bme->humidity();

        meteoLog->add("dsTempOne " + String(dsTempOne));
        meteoLog->add("dsTempTwo " + String(dsTempTwo));
        meteoLog->add("bmePressure " + String(bmePressure));
        meteoLog->add("bmeHum " + String(bmeHum));
        meteoLog->add("Reading sensors... Done");

        if (!isnan(dsTempOne)) {
            sensorsData.dsTempOne = dsTempOne;
        }

        if (!isnan(dsTempTwo)) {
            sensorsData.dsTempTwo = dsTempTwo;
        }

        if (!isnan(bmeHum)) {
            sensorsData.bmeHum = bmeHum;
        }

        if (!isnan(bmePressure)) {
            sensorsData.bmePressure = bmePressure;
        }
    }

    if (millis() - dhtSensorUpdated > DHT_TIMEOUT) {
        dhtSensorUpdated = millis();
        dhtHum = dht->humidity();
        meteoLog->add("dhtHum " + String(dhtHum));

        if (!isnan(dhtHum)) {
            sensorsData.dhtHum = dhtHum;
        }
    }
}

long timeScan = -1e9;
long timeDelta = 1000000;

int cc = 0;

void loop() {
    // Serial.println("loop0 " + String(cc++));
    // Serial1.println("loop1 " + String(cc++));
    otaUpdate.handle();

    if (millis() - timeScan > 2000) {
        timeScan = millis();
        // Serial.println("I2C begin");
        // scan();
        // Serial.println("I2C end");

        // CO2
        int co2ppm = mhz19.getPPM();
        int co2status = mhz19.getStatus();

        meteoLog->add("co2 ppm " + String(co2ppm));
        meteoLog->add("co2 st " + String(co2status));
        // Serial.print("co2: ");
        // Serial.println(co2ppm);
        // Serial.print("temp co2: ");
        // Serial.println(temp);

        long ts = now();
        // RtcDateTime compiled;
        // compiled.InitWithEpoch32Time(ts);
        // Rtc.SetDateTime(compiled);

        if (!Rtc.IsDateTimeValid()) {
            Serial.println("RTC lost confidence in the DateTime!");
        }

        RtcDateTime nowt = Rtc.GetDateTime();
        ts = now();
        timeDelta = ts - nowt.Epoch32Time();
        // Serial.println("Timedelta: " + String(timeDelta));
        // printDateTime(nowt);
        // Serial.println();
    }

    if (oledState == SENSORS) {
        oled->displaySensorsData(sensorsData);
    } else if (oledState == NETWORK) {
        oled->displayIp(cnt++, timeDelta, rtcToString(Rtc.GetDateTime()), NTP.getTimeStr());
        // oled->displayIp(cnt++, NTP.getTimeStr());
    } else if (oledState == LOG) {
        oled->displayLog();
    }

    tryUpdateSensors();

    delay(100);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        if (millis() - timeButtonPress > 500) {
            Serial.println("Button on");

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
