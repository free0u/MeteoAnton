#include "FS.h"
#include "SSD1306.h"
#include "OTAUpdate.h"
#include "WiFiConfig.h"

OTAUpdate otaUpdate;

#include "images.h"
SSD1306 display(0x3C, D3, D5);
#define DEMO_DURATION 3000
typedef void (*Demo)(void);
int demoMode = 0;
int counter = 1;

#define LED D6
#define BUTTON D7
int cnt = 1;

void displayIp(SSD1306 display, int cnt) {
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2,
                       "IP:\n" + WiFi.localIP().toString() + "\n# " + String(cnt));
    display.display();
}

#include "SensorDallasTemp.h"

SensorDallasTemp *temp;

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

// DHT11
#include "DHT.h"
#define DHTPIN D8     // what digital pin we're connected to
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    Serial.println("Booting and setup");

    // setup OLED
    display.init();
    display.flipScreenVertically();
    display.setContrast(255);
    display.setFont(ArialMT_Plain_10);

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);
    // displayIp(display, cnt);

    // configure and start OTA update server
    otaUpdate.setup();

    // DS18B20 temperature
    temp = new SensorDallasTemp();

    // BME280
    if (!bme.begin(0x76)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }

    // DHT11
    dht.begin();

    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    pinMode(D0, OUTPUT);
    pinMode(D4, OUTPUT);

    pinMode(BUTTON, INPUT);

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);
}

// OLED DEMO BEGIN
void drawFontFaceDemo() {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Hello world");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, "Hello world");
    display.setFont(ArialMT_Plain_24);
    display.drawString(0, 26, "Hello world");
}

void drawTextFlowDemo() {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawStringMaxWidth(
        0, 0, 128,
        "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}

void drawTextAlignmentDemo() {
    // Text alignment demo
    display.setFont(ArialMT_Plain_10);

    // The coordinates define the left starting point of the text
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 10, "Left aligned (0,10)");

    // The coordinates define the center of the text
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 22, "Center aligned (64,22)");

    // The coordinates define the right end of the text
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 33, "Right aligned (128,33)");
}

void drawRectDemo() {
    // Draw a pixel at given position
    for (int i = 0; i < 10; i++) {
        display.setPixel(i, i);
        display.setPixel(10 - i, i);
    }
    display.drawRect(12, 12, 20, 20);

    // Fill the rectangle
    display.fillRect(14, 14, 17, 17);

    // Draw a line horizontally
    display.drawHorizontalLine(0, 40, 20);

    // Draw a line horizontally
    display.drawVerticalLine(40, 0, 20);
}

void drawCircleDemo() {
    for (int i = 1; i < 8; i++) {
        display.setColor(WHITE);
        display.drawCircle(32, 32, i * 3);
        if (i % 2 == 0) {
            display.setColor(BLACK);
        }
        display.fillCircle(96, 32, 32 - i * 3);
    }
}

void setBrightnessLed(int);

void drawProgressBarDemo() {
    int progress = (counter) % 100;
    // draw the progress bar
    display.drawProgressBar(0, 32, 120, 10, progress);

    setBrightnessLed(progress * 10 / 3);

    // draw the percentage as String
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 15, String(progress) + "%");
}

void drawImageDemo() {
    // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
    // on how to create xbm files
    display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

Demo demos[] = {drawFontFaceDemo, drawTextFlowDemo,    drawTextAlignmentDemo, drawRectDemo,
                drawCircleDemo,   drawProgressBarDemo, drawImageDemo};
int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;
// OLED DEMO END

bool ledOn = false;

void switchLed() {
    if (ledOn) {
        analogWrite(LED, 0);
    } else {
        analogWrite(LED, counter % 100 * 10 / 3);
    }
    ledOn = !ledOn;
}

void setBrightnessLed(int brightness) {
    // analogWrite(LED, brightness);
    analogWrite(LED, 10);
    //
}

void dimLed() {
    int newDim = cnt * 100 % 1024;
    analogWrite(LED, newDim);
    cnt++;
}

// ENUM
// LED state: OFF, SENSORS, NETWORK

long timeButtonPress = 0;

float tempC = 0;
long tempTime = 0;

int cc = 0;

void loop() {
    if (cc++ % 2 == 0) {
        digitalWrite(D0, HIGH);
        digitalWrite(D4, HIGH);
    } else {
        // digitalWrite(D0, LOW);
        // digitalWrite(D4, LOW);
    }

    otaUpdate.handle();

    display.clear();
    // demos[demoMode]();
    demos[5]();
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(128, 0, String(millis() / 1000 / 5));

    if (millis() - tempTime > 15000) {
        tempTime = millis();
        tempC = temp->printTemperature();
    }

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, String(tempC));

    display.display();

    if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
        // demoMode = (demoMode + 1) % demoLength;
        timeSinceLastModeSwitch = millis();
    }
    counter++;

    // BME 280 begin
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    // BME 280 end

    // DHT11 BEGIN
    float humidity = dht.readHumidity();
    Serial.print("DHT11 hum: ");
    Serial.println(humidity);
    // DHT11 END

    delay(1000);
    int buttonState = digitalRead(BUTTON);
    if (buttonState == HIGH) {
        Serial.println("Button on");
        switchLed();

        if (millis() - timeButtonPress > 500) {
            timeButtonPress = millis();
            // demoMode = (demoMode + 1) % demoLength;
        }
    } else {
        // Serial.println("Button off");
    }

    // IF button LONG PRESS and UPTIME smaller 1 minute
    //     start WiFi Manager

    // IF OLED works longer 1 minute
    //     OLED state -> OFF
    // IF button PUSHED
    //     OLED state -> NEXT
}

//  /**
//   * The MIT License (MIT)
//   *
//   * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
//   * Copyright (c) 2018 by Fabrice Weinberg
//   *
//   * Permission is hereby granted, free of charge, to any person obtaining a
//   copy
//   * of this software and associated documentation files (the "Software"), to
//   deal
//   * in the Software without restriction, including without limitation the
//   rights
//   * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   * copies of the Software, and to permit persons to whom the Software is
//   * furnished to do so, subject to the following conditions:
//   *
//   * The above copyright notice and this permission notice shall be included
//   in all
//   * copies or substantial portions of the Software.
//   *
//   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//   OR
//   * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//   THE
//   * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//   FROM,
//   * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE
//   * SOFTWARE.
//   *
//   * ThingPulse invests considerable time and money to develop these open
//   source libraries.
//   * Please support us by buying our products (and not the clones) from
//   * https://thingpulse.com
//   *
//   */

//  // Include the correct display library
//  // For a connection via I2C using Wire include
//  #include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
//  #include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
//  // or #include "SH1106.h" alis for `#include "SH1106Wire.h"`
//  // For a connection via I2C using brzo_i2c (must be installed) include
//  // #include <brzo_i2c.h> // Only needed for Arduino 1.6.5 and earlier
//  // #include "SSD1306Brzo.h"
//  // #include "SH1106Brzo.h"
//  // For a connection via SPI include
//  // #include <SPI.h> // Only needed for Arduino 1.6.5 and earlier
//  // #include "SSD1306Spi.h"
//  // #include "SH1106SPi.h"

//  // Use the corresponding display class:

//  // Initialize the OLED display using SPI
//  // D5 -> CLK
//  // D7 -> MOSI (DOUT)
//  // D0 -> RES
//  // D2 -> DC
//  // D8 -> CS
//  // SSD1306Spi        display(D0, D2, D8);
//  // or
//  // SH1106Spi         display(D0, D2);

//  // Initialize the OLED display using brzo_i2c
//  // D3 -> SDA
//  // D5 -> SCL
//  // SSD1306Brzo display(0x3c, D3, D5);
//  // or
//  // SH1106Brzo  display(0x3c, D3, D5);

//  // Initialize the OLED display using Wire library
//  SSD1306  display(0x3C, D3, D5);
//  // SH1106 display(0x3c, D3, D5);

// // Adapted from Adafruit_SSD1306
// void drawLines() {
//   for (int16_t i=0; i<display.getWidth(); i+=4) {
//     display.drawLine(0, 0, i, display.getHeight()-1);
//     display.display();
//     delay(10);
//   }
//   for (int16_t i=0; i<display.getHeight(); i+=4) {
//     display.drawLine(0, 0, display.getWidth()-1, i);
//     display.display();
//     delay(10);
//   }
//   delay(250);

//   display.clear();
//   for (int16_t i=0; i<display.getWidth(); i+=4) {
//     display.drawLine(0, display.getHeight()-1, i, 0);
//     display.display();
//     delay(10);
//   }
//   for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
//     display.drawLine(0, display.getHeight()-1, display.getWidth()-1, i);
//     display.display();
//     delay(10);
//   }
//   delay(250);

//   display.clear();
//   for (int16_t i=display.getWidth()-1; i>=0; i-=4) {
//     display.drawLine(display.getWidth()-1, display.getHeight()-1, i, 0);
//     display.display();
//     delay(10);
//   }
//   for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
//     display.drawLine(display.getWidth()-1, display.getHeight()-1, 0, i);
//     display.display();
//     delay(10);
//   }
//   delay(250);
//   display.clear();
//   for (int16_t i=0; i<display.getHeight(); i+=4) {
//     display.drawLine(display.getWidth()-1, 0, 0, i);
//     display.display();
//     delay(10);
//   }
//   for (int16_t i=0; i<display.getWidth(); i+=4) {
//     display.drawLine(display.getWidth()-1, 0, i, display.getHeight()-1);
//     display.display();
//     delay(10);
//   }
//   delay(250);
// }

// // Adapted from Adafruit_SSD1306
// void drawRect(void) {
//   for (int16_t i=0; i<display.getHeight()/2; i+=2) {
//     display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
//     display.display();
//     delay(10);
//   }
// }

// // Adapted from Adafruit_SSD1306
// void fillRect(void) {
//   uint8_t color = 1;
//   for (int16_t i=0; i<display.getHeight()/2; i+=3) {
//     display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
//     display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() -
//     i*2); display.display(); delay(10); color++;
//   }
//   // Reset back to WHITE
//   display.setColor(WHITE);
// }

// // Adapted from Adafruit_SSD1306
// void drawCircle(void) {
//   for (int16_t i=0; i<display.getHeight(); i+=2) {
//     display.drawCircle(display.getWidth()/2, display.getHeight()/2, i);
//     display.display();
//     delay(10);
//   }
//   delay(1000);
//   display.clear();

//   // This will draw the part of the circel in quadrant 1
//   // Quadrants are numberd like this:
//   //   0010 | 0001
//   //  ------|-----
//   //   0100 | 1000
//   //
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2,
//   display.getHeight()/4, 0b00000001); display.display(); delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2,
//   display.getHeight()/4, 0b00000011); display.display(); delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2,
//   display.getHeight()/4, 0b00000111); display.display(); delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2,
//   display.getHeight()/4, 0b00001111); display.display();
// }

// void printBuffer(void) {
//   // Initialize the log buffer
//   // allocate memory to store 8 lines of text and 30 chars per line.
//   display.setLogBuffer(5, 30);

//   // Some test data
//   const char* test[] = {
//       "Hello",
//       "World" ,
//       "----",
//       "Show off",
//       "how",
//       "the log buffer",
//       "is",
//       "working.",
//       "Even",
//       "scrolling is",
//       "working"
//   };

//   for (uint8_t i = 0; i < 11; i++) {
//     display.clear();
//     // Print to the screen
//     display.println(test[i]);
//     // Draw it to the internal screen buffer
//     display.drawLogBuffer(0, 0);
//     // Display it on the screen
//     display.display();
//     delay(500);
//   }
// }

// void setup() {
//   display.init();

//   // display.flipScreenVertically();

//   display.setContrast(255);

//   drawLines();
//   delay(1000);
//   display.clear();

//   drawRect();
//   delay(1000);
//   display.clear();

//   fillRect();
//   delay(1000);
//   display.clear();

//   drawCircle();
//   delay(1000);
//   display.clear();

//   printBuffer();
//   delay(1000);
//   display.clear();
// }

// void loop() { }
