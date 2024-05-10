# Bus Station Data Box - LoRa Gateway

## Overview
The LoRa Gateway component receives data from multiple LoRa Nodes and sends it to the server via Internet.

## Setup
1. **PlatformIO Installation**  
   Install PlatformIO and the required libraries.

2. **Dependencies**  
   - Platform: espressif32
   - Board: esp32dev
   - Library:
     + LoRa_E32.h
     + SoftwareSerial.h
     + AsyncTCP.h
     + ESPAsyncWebServer.h
     + ESPAsyncWebServer.h
     + ArduinoWebsockets.h

   Ensure your `platformio.ini` file has the appropriate dependencies.

3. **Wiring**
   - Module Lora:
     + PIN_M0      Pin 18
     + PIN_M1      Pin 5
     + PIN_AUX     Pin 4

4. **Configuration**
   Read the `.h` file for configuration information.

## Usage
1. **Upload Firmware**  
   Upload the firmware to the gateway using PlatformIO.

2. **Operation**  
   The gateway should start receiving data from nodes and relaying it to the server.

3. **Troubleshooting**
   - Attach the lora antenna and make sure to configure the pins of the lora module if data is not received.
   - Check network connectivity settings.
