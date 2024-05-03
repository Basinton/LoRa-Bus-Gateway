#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>
#include <WiFi.h>
#include <LoRa_E32.h>
#include <SoftwareSerial.h>
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <Arduino_JSON.h>
// #include <SPIFFS.h>


extern char serial_buffer[50]; // Adjust the size as necessary

typedef enum
{
    BUS_UNKNOWN = -1,
    BUS_00,
    BUS_01,
    BUS_02,
    BUS_03,
    BUS_04,
    BUS_COUNT
} BUS_ID;

extern BUS_ID busID;

#endif