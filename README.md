# EnviroSensor

This project explores the viability of a small, cheap, and self contained sensor platform as well as different environmental sensors and different communication and data uploading methods. It was initiated by the [MakitZone](https://makeit.zone/) as "An introduction to DIY automated environmental monitoring technologies" workshop, but we want to take it to the next level!

## Scope
This part of the project focuses on 3 main aspects
1. Using an Heltech ESP32 LoRa Board as a platform for receiving data from sensors
2. Connecting said platform to a network (and in the future a long distance LoRa WAN)
3. Sending the collected data to a remote pub/sub system
4. (Future) Battery saving mode - for field (battary) operated sensors

A seperate subproject will be looking at receiveing the data from the pub/sub systesm and adding functionality such as data presentation, sensor and user registration, etc.

### 1. Sensor Platform
The foundation of this project lies in configuring the Arduino platform to seamlessly connect with our sensors, read data, and present it either on an LCD screen or in the serial monitor. The key to making this platform a low barrier and user friendly solution is to enable addition and removal of sensors without having to hard code each one.
This feature will be added in future iteration of the code.

We prototyped it with a temperature and humidity sensor (BME280). The goal is to continually add more sensors to this list.

Current supported boards:
* Heltech ESP32 ESP-32S LoRa V 2.1 868MHz-915MHz 0.96 OLED Display WiFi Bluetooth Development Board Antenna Transceiver SX1276

Current knowen supported sensors:
#### Temperature
BME280 (also a humidity sensor)

#### Humidity
BME280 (also a temperature sensor)

### 2. Network Connectivity
Our end goal is to use a LoRaWan and be able to send data over longer distances and in short bursts to conserve battery.
For setup and utility, we added the ability to connect the ESP32 board to different Access Points (networks) without hard-coding network credentials (SSID and password). Your ESP will automatically join the last saved network or set up an Access Point that you can use to configure the network credentials.
In order to store SSID information (and not hardcode it), we added the SPIFFS (File System).
Since the LoRaWan and possibly other network connections might not be as reliable, we will have to store some of the sensor data on the platform for a short time, utilizing the file system as well.

### 3. Sending Data to a Server
In order to have more functionality than just viewing current sensor readings, we need to send the data somewhere.
Some potential usage of the data:
1.  Smart Home systems (Home Assistant) - turn fan on/off, warning messages, etc.
2.  Present single sensor data over time/avarages/etc.
3.  Present multiple sensor data over time/space

For all of these usages we simply connect our platform to a pub/sub system, decoupling the source from the destination and keeping this open to the user.

## Based on
This project, started as a workshop, was based on a few turorials and examples created by others:
1.  Rui Santos - Complete roject details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
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

