# EnviroSensor

This project explores the viability of a small, cheap, and self contained sensor platform as well as different environmental sensors and different communication and data uploading methods. It was initiated by the [MakitZone](https://makeit.zone/) as "An introduction to DIY automated environmental monitoring technologies" workshop, but we want to take it to the next level!

## Scope
This part of the project focuses on 3 main aspects
1. Using an Heltech ESP32 LoRa Board as a platform for receiving data from sensors
2. Connecting said platform to a network (and in the future a long distance LoRa WAN)
3. Sending the collected data to a remote pub/sub system

A seperate subproject will be looking at receiveing the data from the pub/sub systesm and adding functionality such as data presentation, sensor and user registration, etc.

## Based on
This project, started as a workshop, was based on a few turorials and examples created by others:
1.  Rui Santos - Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
2.  Rui Santos - Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/

## How To's (AKA getting it working)
Setting up the USB driver [here](https://docs.heltec.org/general/establish_serial_connection.html)

Setting up Arduino for use with the Heltech ESP32 LoRa Board [here](https://docs.heltec.org/en/node/esp32/index.html)

Setting up SPIFFS file system uploader in Arduino [here](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/) 
**(At the moment, this is not compatible with Arduino 2.0.)**
If you choose to use Arduino IDE, upload files with an older version of Arduino compiler, and use the newer one for your regular code editing/compiling.

## Libraries Needed
[ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer/archive/refs/heads/master.zip) (.zip folder)

[AsyncTCP](https://github.com/me-no-dev/AsyncTCP/archive/refs/heads/master.zip) (.zip folder)

[WifiManager](https://github.com/tzapu/WiFiManager)

