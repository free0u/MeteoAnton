#include "FS.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "WiFiConfig.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Booting and setup");

    // setup wifi connection
    // if cant connect to wifi, configurator will be started
    // @todo: start configurator by pressing button, elsewhere just stay disconnected
    WiFiConfig wifiConfig;
    wifiConfig.connectWiFi(false);

    

    

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("NodeMCU-OTA");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

    // Serial.print("SPIFFS: ");
    // Serial.println(SPIFFS.begin());
    // FSInfo fs_info;
    // SPIFFS.info(fs_info);
    // Serial.println(fs_info.totalBytes);
    // Serial.println(fs_info.usedBytes);
}

void loop() {
  ArduinoOTA.handle();
  delay(1000);
  Serial.println("Hi");
}


//  /**
//   * The MIT License (MIT)
//   *
//   * Copyright (c) 2018 by ThingPulse, Daniel Eichhorn
//   * Copyright (c) 2018 by Fabrice Weinberg
//   *
//   * Permission is hereby granted, free of charge, to any person obtaining a copy
//   * of this software and associated documentation files (the "Software"), to deal
//   * in the Software without restriction, including without limitation the rights
//   * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   * copies of the Software, and to permit persons to whom the Software is
//   * furnished to do so, subject to the following conditions:
//   *
//   * The above copyright notice and this permission notice shall be included in all
//   * copies or substantial portions of the Software.
//   *
//   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//   * SOFTWARE.
//   *
//   * ThingPulse invests considerable time and money to develop these open source libraries.
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
//     display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() - i*2);
//     display.display();
//     delay(10);
//     color++;
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
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000001);
//   display.display();
//   delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000011);
//   display.display();
//   delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000111);
//   display.display();
//   delay(200);
//   display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00001111);
//   display.display();
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
