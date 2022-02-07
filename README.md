[![CircleCI](https://circleci.com/gh/free0u/MeteoFirmware/tree/master.svg?style=shield&circle-token=b15ceaee303d411a0b91a3e03d85fda2e4fda699)](https://circleci.com/gh/free0u/MeteoFirmware/tree/master)

## Description

Firmware for my meteo (historically) devices, which have temperature, humidity, co2, power spent, water spent sensors connected.
About 10 devices are online in 4 different locations. First device with this firmware created in 2018.

Data is sent to my server and viewed by Grafana dashboards.
Public dashboard with outside temperature available here: [Grafana](https://meteo-anton.tk/d/YwmA7p-7z/public)

Typical device based on esp8266 chip (kind of Arduino with WiFi) and looks like this.
<img src="https://user-images.githubusercontent.com/2528711/152792263-ff3405ce-c024-4c09-b02c-80450b4f9ebb.jpg" width="600">

My favorite part is that all devices have identical firmware and load its configuration on start [based on chipId](https://github.com/free0u/MeteoFirmware/blob/master/src/system/DevicesConfig.h#L638) (at beginning of project I maintain different firmware for every device and it was a lot of headache 🤯).