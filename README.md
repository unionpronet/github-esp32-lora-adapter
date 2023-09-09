# Introduction 
This project demonstrates how to leverage ESP32 to forward a Lora message to an Android device via Bluetooth Classic.

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/LICENSE)

<a href="https://www.buymeacoffee.com/unionpro" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 38px !important;width: 168px !important;" ></a>


## System block diagram
```
    Lora transmitter                                            Lora receiver                    Android logger

   +----------------+                                         +----------------+
   |   Power bank   |                                         |   Power bank   |
   +----------------+                                         +----------------+
           |                                                          |
   +----------------+                                         +----------------+                 +-----------+
   |   ESP32 Lora   | +------+                       +------+ |   ESP32 Lora   |   (Bluetooth)   |  mobile   |
   |    adapter     |-| Lora |    ...............    | Lora |-|    adapter     |       ...       |  device   |
   +----------------+ +------+                       +------+ +----------------+                 +-----------+
```

## Hardware requirement
1. Two pieces of "Doit ESP32 DevKit V1"
2. Two pieces of [Ai-Thinker Ra-01SC](https://www.lcsc.com/product-detail/LoRa-Modules_Ai-Thinker-Ra-01SC_C2830749.html)
3. Two pieces of [esp32-lora-adapter](https://unionpronet.wordpress.com/esp32-lora-adapter/)

## Software setup
1. Install ESP32 in Arduino IDE (https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
2. Install Arduino [DebugLog lib](https://www.arduino.cc/reference/en/libraries/debuglog/)
3. Install Arduino [RadioLib](https://www.arduino.cc/reference/en/libraries/radiolib/)
4. Install [Arduino MemStream lib](https://github.com/Apollon77/MemStream)
5. Download the project zip file from this repository and extract it under your Arduino project folder

## Modify MemStream library
modify header files for ESP32 (line 26 of <Arduino project folder>/libraries/MemStream/MemStream.h)

From
```
#include <inttypes.h>
#include <Stream.h>
#include <avr/pgmspace.h>
```
To
```
#include <inttypes.h>
#include <Stream.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#elif defined(XMC1300_Boot_Kit)
#undef memcpy_P
#define memcpy_P memcpy
#else
#define pgm_read_byte(addr) \
  (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR
#endif
```

## Build and upload firmware (Lora receiver)
1. Launch Arduino IDE
2. Open this project (esp32-lora-adapter.ino)
3. Click menu "Sketch" -> "Verify/Compile". If everything goes smooth, you should see the followings
[![Build success](/images/build-success.jpg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/images/build-success.jpg)

4. Put a "Doit ESP32 DevKit V1" on "esp32-lora-adapter" 

[![TO BE available!!! esp32-lora-adapter image](/images/esp32-lora-adapter.jpg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/images/esp32-lora-adapter.jpg)

5. Connect the "Doit ESP32 DevKit V1" to PC by an USB cable
6. On the Arduino IDE, click menu "Sketch" -> "Upload". You should see the followings
[![Upload success](/images/upload-success.jpg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/images/upload-success.jpg)

7. Launch "Serial Monitor" in the Arduino IDE by clicking menu "Tools" -> "Serial Monitor". You should see the followings
[![Serial monitor](/images/serial-monitor.jpg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/images/serial-monitor.jpg)



## Android App Installation
1. Install [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US) on an Android device
2. [Pair ESP32 on Android device](https://randomnerdtutorials.com/esp32-bluetooth-classic-arduino-ide/#:~:text=Press%20the%20ESP32%20Enable%20button,ve%20enable%20your%20smartphone's%20Bluetooth.)

## Test
1.  Connect "ESP32" and "Android Serial Bluetooth Terminal" (https://randomnerdtutorials.com/esp32-bluetooth-classic-arduino-ide/#:~:text=Press%20the%20ESP32%20Enable%20button,ve%20enable%20your%20smartphone's%20Bluetooth.)
2.  Repeat the "Build and upload firmware" steps above on another set of hardware (Lora transmitter)
3.  Send a Lora message by clicking the button "BOOT" on another "Doit ESP32 DevKit V1". If everything goes smooth, you should see the followings on the Android device.
[![ESP32 Lora Adapter](/images/bluetooth-serial-screen.jpg)](https://github.com/unionpronet/github-esp32-lora-adapter/blob/main/images/bluetooth-serial-screen.jpg)
---

### Please refer to "arduino-esp32-lora-adapter.ino" for complete code
## *** source code will be available in B/Nov ***

### Video demo is available on  
[xmc-lora-sensor video](https://youtu.be/yTs0bI0mfuo)  
Setup Lora Receiver and Android APP during 0 - 8s.  
Power on the "Solar Power Lora Sensor" on 9s.  
The first sensors' data is received at 28s, and repeatedly receive every 5 seconds.


# Contribute
Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this project.

If you want to contribute to this project:
- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

### License
- The library is licensed under GNU LESSER GENERAL PUBLIC LICENSE Version 3
---

### Copyright
- Copyright 2023 unionpro.net@gmail.com. All rights reserved.
