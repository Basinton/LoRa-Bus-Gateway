#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>

extern String second_ip;
extern int isConnect;


struct BUS
{
    uint8_t busRoute;
    String busDriverName;
    uint16_t messageID;

    float busLat;
    float busLong;
    double busSpeed;
    float busDistance;

    uint8_t busDirection;
    uint8_t nowBusStop;
    uint8_t preBusStop;
};

extern BUS bus50;
extern BUS bus08;

#endif